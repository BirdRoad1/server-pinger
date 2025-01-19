#include "cidr.hpp"
#include <sstream>

unsigned int ipToInt(const std::string &ip)
{
    int result = 0;
    std::istringstream iss(ip);
    std::string token;
    for (int i = 0; i < 4; ++i)
    {
        std::getline(iss, token, '.');
        result = (result << 8) + std::stoi(token);
    }
    return result;
}

// Convert a 32-bit integer back to an IPv4 string
std::string intToIp(const unsigned int ip)
{
    return std::to_string((ip >> 24) & 0xFF) + "." +
           std::to_string((ip >> 16) & 0xFF) + "." +
           std::to_string((ip >> 8) & 0xFF) + "." +
           std::to_string(ip & 0xFF);
}

// Calculate the ending IP of a CIDR range
unsigned int cidrEndIp(const unsigned int startIp, const unsigned int prefixLength)
{
    int mask = (prefixLength == 0) ? 0 : (~0U << (32 - prefixLength));
    return startIp | ~mask;
}