#pragma once
#include <mutex>

class Stats
{
private:
    std::mutex ipsDoneMutex;
    std::mutex ipsTotalMutex;
    std::mutex packetsMutex;
    long long ipsDone;
    long long ipsTotal;
    long long packets;

public:
    long long getIpsDone();
    void setIpsDone(long long ipsDone);

    long long getIpsTotal();
    void setIpsTotal(long long ipsTotal);

    long long getPackets();
    void setPackets(long long packets);
};