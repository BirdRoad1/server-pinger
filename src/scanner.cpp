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
#include "progress_writer.hpp"

std::queue<CidrRange> ipQueue;
std::mutex mut;
Stats stats;

void scanTask(Database *db, const unsigned int ip) {
    std::string ipStr = intToIp(ip);
    std::string dataStr;

    try {
        dataStr = pingServer(ipStr, 25565);
        ServerData data = parseServerData(ipStr, 25565, dataStr);
        std::cout << "Working:" << data.host << std::endl;
        db->insertServerData(data);
    } catch (net_exception &ex) {
        // std::cout << "Net exception: " << ex.what() << std::endl;
    }
    catch (ping_parse_exception &ex) {
        // std::cout << "Ping parse exception: " << ex.what() << ", data: " << dataStr << std::endl;
    }
    catch (std::exception &ex) {
        // std::cout << "Caught exception while checking server: " << ex.what() << std::endl;
    }

    // ProgressWriter::writeIP(ipStr);
    stats.setIpsDone(stats.getIpsDone() + 1);
    stats.setPackets(stats.getPackets() + 10);
}

void runThread(Database *db) {
    while (true) {
        std::vector<CidrRange> batch;
        std::unique_lock lock(mut);

        if (ipQueue.empty()) {
            mut.unlock();
            break;
        }

        for (int i = 0; i < 50 && !ipQueue.empty(); i++) {
            batch.push_back(ipQueue.front());
            ipQueue.pop();
        }

        lock.unlock();

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
        const long long packets = stats.getPackets();
        const long long threads = stats.getActiveThreads();

        const double fraction = total == 0
                                    ? -1
                                    : static_cast<double>(stats.getIpsDone()) / static_cast<double>(stats.getIpsTotal())
                                      * 100;
        stats.setPackets(0);

        std::cout << std::fixed << std::setprecision(2) << "Done: " << ips << " / " << total << " (" << fraction <<
                "%, " << packets << " PPS, " << threads << " threads)" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
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
            std::cout << "Could not convert bitmask to int: " << ex.what() << std::endl;
            return;
        }

        ipQueue.push(CidrRange{ipToInt(ip), cidrEndIp(ipToInt(ip), bitmask)});
    }
}

void startScanner(Database *db, long numThreads) {
    // Read and parse CIDRs
    std::ifstream file("cidrs.txt");
    if (!file.good()) {
        std::cout << "Failed to open cidrs.txt" << std::endl;
        return;
    }

    // std::vector<std::string> lines;

    std::string line;
    while (std::getline(file, line)) {
        handleFileLine(line);
    }


    std::vector<std::thread> threads;

    std::cout << "Loaded " << ipQueue.size() << " IP addresses" << std::endl;
    std::cout << "Starting in 3 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    stats.setIpsTotal(ipQueue.size());

    // Start stats printer thread
    std::thread(statsThread).detach();

    for (long i = 0; i < numThreads; i++) {
        threads.emplace_back(std::thread(runThread, db));
    }

    std::cout << "Created " << threads.size() << " threads" << std::endl;
    stats.setActiveThreads(threads.size());

    while (!threads.empty()) {
        std::thread &t = threads.front();
        if (t.joinable()) {
            t.join();
        }
    }

    ProgressWriter::close();
    std::cout << "Done scanning!" << threads.size() << std::endl;
}
