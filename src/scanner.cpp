#include "scanner.hpp"

#include <iostream>
#include "cidr.hpp"
#include <fstream>
#include <string>
#include "mcping.hpp"
#include <thread>
#include <iomanip>
#include "stats.hpp"
#include "serverdata.hpp"
#include "exception/net_exception.hpp"
#include "exception/ping_parse_exception.hpp"
#include <queue>
#include "logger.hpp"
#include "progress_writer.hpp"

std::queue<CidrRange> ipQueue;
std::mutex mut;
Stats stats;

void scanTask(Database *db, const unsigned int ip) {
    const std::string ipStr = intToIp(ip);

    try {
        const std::string dataStr = pingServer(ipStr, 25565);
        const ServerData data = parseServerData(ipStr, 25565, dataStr);
        logger->info()->write("Working: ")->write(data.host)->end();
        db->insertServerData(data);
    } catch (net_exception &ex) {
        // std::cout << "Net exception: " << ex.what() << std::endl;
    }
    catch (ping_parse_exception &ex) {
        // std::cout << "Ping parse exception: " << ex.what() << ", data: " << dataStr << std::endl;
    }
    catch (std::exception &ex) {
        logger->error()->write("Caught exception while checking server: ")->write(ex.what())->end();
    }

    // ProgressWriter::writeIP(ipStr);
    stats.setIpsDone(stats.getIpsDone() + 1);
    stats.setPackets(stats.getPackets() + 1);
}

void runThread(Database *db) {
    while (true) {
        std::vector<CidrRange> batch;
        std::unique_lock lock(mut);

        if (ipQueue.empty()) {
            mut.unlock();
            break;
        }

        for (int i = 0; i < 100 && !ipQueue.empty(); i++) {
            batch.push_back(ipQueue.front());
            ipQueue.pop();
        }

        lock.unlock();

        if (batch.empty()) break;

        for (CidrRange range: batch) {
            if (range.startIp == range.endIp) {
                // one ip
                scanTask(db, range.startIp);
            } else {
                for (unsigned int i = range.startIp; i < range.endIp; i++) {
                    scanTask(db, i);
                }
            }
        }
    }

    stats.setActiveThreads(stats.getActiveThreads() - 1);
}

/**
 * Prints scanner progress periodically
 */
[[noreturn]] void statsThread() {
    while (true) {
        const long long ips = stats.getIpsDone();
        const long long total = stats.getIpsTotal();
        const long long packets = stats.getPackets() / 5;
        const long long threads = stats.getActiveThreads();

        const double fraction = total == 0
                                    ? -1
                                    : static_cast<double>(stats.getIpsDone()) / static_cast<double>(stats.
                                          getIpsTotal());
        stats.setPackets(0);

        logger->info()->fixed()->setPrecision(2)->write("Done: ")->write(ips)->write(" / ")->write(total)->write(" (")->
                write(fraction)->
                write("%, ")->write(packets)->write(" CPS, ")->write(threads)->write(" threads)")->end();

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void handleFileLine(const std::string &line) {
    if (line.starts_with("#"))
        return;

    std::size_t pos = line.find('/');
    if (pos == std::string::npos) {
        // Handle IP address
        ipQueue.push(CidrRange{ipToInt(line), ipToInt(line)});
    } else {
        // Handle CIDR
        const std::string ip = line.substr(0, pos);
        const std::string bitmaskStr = line.substr(pos + 1);

        int bitmask;
        try {
            bitmask = std::stoi(bitmaskStr);
        } catch (std::exception &ex) {
            logger->warn()->write("Could not convert bitmask to int: ")->write(ex.what())->end();
            return;
        }

        ipQueue.push(CidrRange{ipToInt(ip), cidrEndIp(ipToInt(ip), bitmask)});
    }
}

void startScanner(Database *db, long numThreads) {
    // Read and parse CIDRs
    std::ifstream file("cidrs.txt");
    if (!file.good()) {
        logger->error()->write("Failed to open cidrs.txt")->end();
        return;
    }

    logger->info()->write("Reading IP addresses...")->end();
    std::string line;
    while (std::getline(file, line)) {
        handleFileLine(line);
    }


    std::vector<std::thread> threads;

    logger->info()->write("Loaded ")->write(ipQueue.size())->write(" IP addresses")->end();
    logger->info()->write("Starting in 3 seconds...")->end();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    stats.setIpsTotal(ipQueue.size());

    // Start stats printer thread
    std::thread(statsThread).detach();

    for (long i = 0; i < numThreads; i++) {
        threads.emplace_back(runThread, db);
    }

    logger->info()->write("Created ")->write(threads.size())->write(" threads")->end();
    stats.setActiveThreads(threads.size());

    while (!threads.empty()) {
        std::thread &t = threads.front();
        if (t.joinable()) {
            t.join();
        }
    }

    ProgressWriter::close();
    logger->info()->write("Done scanning!")->write(threads.size())->end();
}
