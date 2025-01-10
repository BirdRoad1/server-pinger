#pragma once
#include <string>

/**
 * Send Server List Ping request to server and returns the response
 * If there is no response or something goes wrong, `net_exception` is thrown
 */
std::string pingServer(std::string host, int port);