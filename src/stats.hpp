#pragma once
#include <mutex>

class Stats
{
private:
    std::mutex ipsDoneMutex;
    std::mutex ipsTotalMutex;
    std::mutex packetsMutex;
    long long ipsDone = 0;
    long long ipsTotal = 0;
    long long packets = 0;

public:
    long long getIpsDone();
    void setIpsDone(long long ipsDone);

    long long getIpsTotal();
    void setIpsTotal(long long ipsTotal);

    long long getPackets();
    void setPackets(long long packets);
};