#include "scanner.hpp"

#include <iostream>
#include "cidr.hpp"
#include <fstream>
#include <string>
#include "mcping.hpp"
#include <thread>
#include <iomanip>
#include "stats.hpp"
#include <optional>
#include "serverdata.hpp"
#include "exception/net_exception.hpp"
#include "exception/ping_parse_exception.hpp"
#include <queue>

#define THREADS 300

std::queue<CidrRange> ipQueue;
std::mutex mut;
Stats stats;

void scanTask(Database *db, int ip)
{
    std::string ipStr = intToIp(ip);
    std::string dataStr;

    try
    {
        dataStr = pingServer(ipStr, 25565);
        ServerData data = parseServerData(ipStr, 25565, dataStr);
        std::cout << "MOTD:" << data.descriptionJson << std::endl;
        db->insertServerData(data);
    }
    catch (net_exception &ex)
    {
        // std::cout << "Net exception: " << ex.what() << std::endl;
    }
    catch (ping_parse_exception &ex)
    {
        std::cout << "Ping parse exception: " << ex.what() << ", data: " << dataStr << std::endl;
    }
    catch (std::exception &ex)
    {
        std::cout << "Caught exception while checking server: " << ex.what() << std::endl;
    }

    stats.setIpsDone(stats.getIpsDone() + 1);
    stats.setPackets(stats.getPackets() + 1);
}

void runThread(Database *db)
{
    std::cout << "Thread started!" << std::endl;
    while (true)
    {
        mut.lock();
        CidrRange range = ipQueue.front();
        if (ipQueue.empty())
        {
            mut.unlock();
            return;
        }

        ipQueue.pop();
        mut.unlock();

        if (range.startIp == range.endIp)
        {
            // one ip
            scanTask(db, range.startIp);
        }
        else
        {
            for (unsigned int i = range.startIp; i < range.endIp; i++)
            {
                scanTask(db, i);
            }
        }
    }

    std::cout << "Thread shutdown!" << std::endl;
}

/**
 * Prints scanner progress periodically
 */
void statsThread()
{
    while (true)
    {
        long long ips = stats.getIpsDone();
        long long total = stats.getIpsTotal();
        int packets = stats.getPackets();

        double fraction = total == 0 ? -1 : (((double)stats.getIpsDone() / (double)stats.getIpsTotal()) * 100);

        std::cout << std::fixed << std::setprecision(2) << "Done: " << ips << " / " << total << " (" << fraction << "%, " << packets << " PPS)" << std::endl;
        stats.setPackets(0);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void handleFileLine(std::string line)
{
    if (line.starts_with("#"))
        return;

    std::size_t pos = line.find("/");
    if (pos == std::string::npos)
    {
        // Handle IP address
        ipQueue.push(CidrRange{ipToInt(line), ipToInt(line)});
    }
    else
    {
        // Handle CIDR
        std::string ip = line.substr(0, pos);
        std::string bitmaskStr = line.substr(pos + 1);

        int bitmask;
        try
        {
            bitmask = std::stoi(bitmaskStr);
        }
        catch (std::exception &ex)
        {
            std::cout << "Could not convert bitmask to int: " << ex.what() << std::endl;
            return;
        }

        ipQueue.push(CidrRange{ipToInt(ip), cidrEndIp(ipToInt(ip), bitmask)});
    }
}

void startScanner(Database *db)
{
    // Read and parse CIDRs
    std::ifstream file("cidrs.txt");
    if (!file.good())
    {
        std::cout << "Failed to open cidrs.txt" << std::endl;
        return;
    }

    std::vector<std::string> lines;

    std::string line;
    while (std::getline(file, line))
    {
        handleFileLine(line);
    }

    // Start stats printer thread
    std::thread(statsThread).detach();

    std::vector<std::thread> threads;

    for (int i = 0; i < THREADS; i++)
    {
        threads.emplace_back(std::thread(runThread, db));
    }

    stats.setIpsTotal(ipQueue.size());
    std::cout << "Loaded " << ipQueue.size() << " IP addresses" << std::endl;

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "Done scanning!" << std::endl;
}