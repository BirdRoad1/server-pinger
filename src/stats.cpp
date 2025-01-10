#include "stats.h"

int Stats::getIpsDone()
{
    return this->ipsDone;
}

void Stats::setIpsDone(int ipsDone)
{
    statsMutex.lock();
    this->ipsDone = ipsDone;
    statsMutex.unlock();
}

int Stats::getIpsTotal()
{
    return this->ipsTotal;
}

void Stats::setIpsTotal(int ipsTotal)
{
    statsMutex.lock();
    this->ipsTotal = ipsTotal;
    statsMutex.unlock();
}

int Stats::getPackets()
{
    return this->packets;
}

void Stats::setPackets(int packets)
{
    statsMutex.lock();
    this->packets = packets;
    statsMutex.unlock();
}