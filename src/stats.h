#pragma once
#include <mutex>

class Stats
{
private:
    std::mutex statsMutex;
    int ipsDone;
    int ipsTotal;
    int packets;

public:
    int getIpsDone();
    void setIpsDone(int ipsDone);
    
    int getIpsTotal();
    void setIpsTotal(int ipsTotal);
    
    int getPackets();
    void setPackets(int packets);
};