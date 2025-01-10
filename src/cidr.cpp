#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

struct CidrRange
{
    std::string cidr;
    uint32_t startIp;
    uint32_t endIp;
}; // Convert an IPv4 string to a 32-bit integer
uint32_t ipToInt(const std::string &ip)
{
    uint32_t result = 0;
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
std::string intToIp(uint32_t ip)
{
    return std::to_string((ip >> 24) & 0xFF) + "." +
           std::to_string((ip >> 16) & 0xFF) + "." +
           std::to_string((ip >> 8) & 0xFF) + "." +
           std::to_string(ip & 0xFF);
}

// Calculate the ending IP of a CIDR range
uint32_t cidrEndIp(uint32_t startIp, int prefixLength)
{
    uint32_t mask = (prefixLength == 0) ? 0 : (~0U << (32 - prefixLength));
    return startIp | ~mask;
}