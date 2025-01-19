#include "stats.hpp"

long long Stats::getIpsDone() const {
    return this->ipsDone.load();
}

void Stats::setIpsDone(const long long ipsDone)
{
    this->ipsDone.store(ipsDone);
}

long long Stats::getIpsTotal() const {
    return this->ipsTotal.load();
}

void Stats::setIpsTotal(const long long ipsTotal)
{
    this->ipsTotal.store(ipsTotal);
}

long long Stats::getPackets() const {
    return this->packets.load();
}

void Stats::setPackets(long long packets)
{
    this->packets.store(packets);
}

long long Stats::getActiveThreads() const {
    return this->activeThreads.load();
}

void Stats::setActiveThreads(long long packets)
{
    this->activeThreads.store(packets);
}