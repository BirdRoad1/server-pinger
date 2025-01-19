#pragma once
#include <atomic>

class Stats
{
private:
    std::atomic<long long> ipsDone{0};
    std::atomic<long long> ipsTotal{0};
    std::atomic<long long> packets{0};
    std::atomic<long long> activeThreads{0};

public:
    long long getIpsDone() const;
    void setIpsDone(long long ipsDone);

    long long getIpsTotal() const;
    void setIpsTotal(long long ipsTotal);

    long long getPackets() const;
    void setPackets(long long packets);

    long long getActiveThreads() const;
    void setActiveThreads(long long packets);
};