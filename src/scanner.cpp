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

#define THREADS 5000

Stats stats;

void runThread(Database *db, uint32_t startIP, uint32_t endIP)
{
    for (uint32_t i = startIP; i <= endIP; i++)
    {
        std::string ipStr = intToIp(i);
        std::string dataStr;

        try
        {
            dataStr = pingServer(ipStr, 25565);
            ServerData data = parseServerData(ipStr, 25565, dataStr);
            std::cout << "Parsed server: " << ipStr << ", motd: " << data.descriptionJson << std::endl;
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
}

/**
 * Prints scanner progress periodically
 */
void statsThread()
{
    while (true)
    {
        std::cout << std::fixed << std::setprecision(2) << "Done: " << stats.getIpsDone() << " / " << stats.getIpsTotal() << ", PPS: " << stats.getPackets() << "(" << (((double)stats.getIpsDone() / (double)stats.getIpsTotal()) * 100) << "%)" << std::endl;
        stats.setPackets(0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
        lines.push_back(line);
    }

    std::vector<CidrRange> ranges;
    for (std::string line : lines)
    {
        if (line.at(0) == '#')
            continue;

        size_t index = line.find("/");
        if (index < 0)
        {
            std::cout << "Invalid line: " << line << std::endl;
            continue;
        }

        std::string ip = line.substr(0, index);
        std::string bitmaskStr = line.substr(index + 1);

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

        CidrRange range = {line, ipToInt(ip), cidrEndIp(ipToInt(ip), bitmask)};
        ranges.push_back(range);
    }

    // Start stats printer thread
    std::thread(statsThread).detach();

    for (const auto &range : ranges)
    {
        uint32_t numIPs = range.endIp - range.startIp;
        uint32_t ipsPerThread = numIPs / THREADS;
        uint32_t remainder = numIPs % THREADS; // Handle leftover IPs

        uint32_t ip = range.startIp;

        std::vector<std::thread> threads;

        stats.setIpsTotal(numIPs);
        while (ip < range.endIp)
        {
            uint32_t endIP = ip + ipsPerThread - 1;

            // Distribute remainder IPs to first few threads
            if (remainder > 0)
            {
                ++endIP;
                --remainder;
            }

            // Ensure endIP does not exceed the range's end IP
            if (endIP > range.endIp)
            {
                endIP = range.endIp;
            }

            threads.emplace_back(std::thread(runThread, db, ip, endIP));

            ip = endIP + 1; // Move to the next starting IP
        }

        for (auto &t : threads)
        {
            t.join();
        }

        stats.setIpsDone(0);
    }

    std::cout << "Done!" << std::endl;
}