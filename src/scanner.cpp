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

#define THREADS 1000

Stats stats;

void scanTask(Database *db, int ip)
{
    std::string ipStr = intToIp(ip);
    std::string dataStr;

    try
    {
        dataStr = pingServer(ipStr, 25565);
        ServerData data = parseServerData(ipStr, 25565, dataStr);
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

void runThread(Database *db, long long startIP, long long endIP)
{
    std::cout << "Thread started!" << std::endl;

    for (long long i = startIP; i < endIP; i++)
    {
        scanTask(db, i);
    }

    std::cout << "Thread shutdown!" << std::endl;
}

void runThreadVec(Database *db, std::vector<int> ips)
{
    std::cout << "Thread started!" << std::endl;

    for (int i : ips)
    {
        scanTask(db, i);
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

void scanExtraIPs(Database *db, std::vector<int> &extraIPs)
{
    size_t total_ips = extraIPs.size();         // Total number of IPs
    long long chunk_size = total_ips / THREADS; // Base size of each chunk
    long long remainder = total_ips % THREADS;  // Remaining IPs to distribute
    int chunks = chunk_size > 0 ? total_ips / chunk_size : 0;

    std::vector<std::thread> threads;

    stats.setIpsTotal(extraIPs.size());

    for (int i = 0; i < chunks; i++)
    {
        long long start = i * chunk_size;
        long long end = start + chunk_size;

        std::vector<int> slice;
        for (int i = start; i < end; i++)
        {
            slice.push_back(extraIPs.at(i));
        }

        threads.emplace_back(std::thread(runThreadVec, db, slice));
    }

    if (remainder > 0)
    {
        std::vector<int> slice;
        for (size_t i = total_ips - remainder; i < total_ips; i++)
        {
            slice.push_back(extraIPs.at(i));
        }

        threads.emplace_back(std::thread(runThreadVec, db, slice));
    }

    for (auto &t : threads)
    {
        t.join();
    }

    stats.setIpsDone(0);
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

    // IPs in ips.txt that aren't ranges
    std::vector<int> extraIPs;

    std::vector<CidrRange> ranges;
    for (std::string line : lines)
    {
        if (line.at(0) == '#')
            continue;

        size_t index = line.find("/");

        if (index == std::string::npos)
        {
            extraIPs.push_back(ipToInt(line));
        }
        else
        {
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
    }

    // Start stats printer thread
    std::thread(statsThread).detach();

    // Scan individual IPs first
    std::cout << "Loaded " << extraIPs.size() << " individual IPs" << std::endl;
    scanExtraIPs(db, extraIPs);

    // Now, scan CIDRs
    std::cout << "Now scanning ranges!" << std::endl;
    for (const auto &range : ranges)
    {
        long long start_ip = range.startIp; // Example starting IP (as integer)
        long long end_ip = range.endIp;     // Example ending IP (as integer)

        long long total_ips = end_ip - start_ip + 1; // Total number of IPs
        long long chunk_size = total_ips / THREADS;  // Base size of each chunk
        long long remainder = total_ips % THREADS;   // Remaining IPs to distribute
        std::cout << chunk_size << std::endl;
        int chunks = chunk_size > 0 ? total_ips / chunk_size : 0;

        std::vector<std::thread> threads;

        stats.setIpsTotal(total_ips);

        for (int i = 0; i < chunks; i++)
        {
            long long start = i * chunk_size;
            long long end = start + chunk_size;
            threads.emplace_back(std::thread(runThread, db, start, end));
        }

        long long leftOff = (end_ip - remainder + 1);

        if (remainder > 0)
        {
            threads.emplace_back(std::thread(runThread, db, leftOff, end_ip + 1));
        }

        for (auto &t : threads)
        {
            t.join();
        }

        stats.setIpsDone(0);
    }

    std::cout << "Done!" << std::endl;
}