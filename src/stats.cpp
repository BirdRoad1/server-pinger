#include "stats.hpp"

long long Stats::getIpsDone()
{
    return this->ipsDone;
}

void Stats::setIpsDone(long long ipsDone)
{
    ipsDoneMutex.lock();
    this->ipsDone = ipsDone;
    ipsDoneMutex.unlock();
}

long long Stats::getIpsTotal()
{
    return this->ipsTotal;
}

void Stats::setIpsTotal(long long ipsTotal)
{
    ipsTotalMutex.lock();
    this->ipsTotal = ipsTotal;
    ipsTotalMutex.unlock();
}

long long Stats::getPackets()
{
    return this->packets;
}

void Stats::setPackets(long long packets)
{
    packetsMutex.lock();
    this->packets = packets;
    packetsMutex.unlock();
}