#pragma once
#include <string>

struct CidrRange
{
    std::string cidr;
    uint32_t startIp;
    uint32_t endIp;
};

// Convert an IPv4 string to a 32-bit integer
uint32_t ipToInt(const std::string &ip);

// Convert a 32-bit integer back to an IPv4 string
std::string intToIp(uint32_t ip);

// Calculate the ending IP of a CIDR range
uint32_t cidrEndIp(uint32_t startIp, int prefixLength);