#pragma once
#include <string>

struct CidrRange
{
    std::string cidr;
    unsigned int startIp;
    unsigned int endIp;
};

// Convert an IPv4 string to a 32-bit integer
unsigned int ipToInt(const std::string &ip);

// Convert a 32-bit integer back to an IPv4 string
std::string intToIp(unsigned int ip);

// Calculate the ending IP of a CIDR range
unsigned int cidrEndIp(unsigned int startIp, unsigned int prefixLength);