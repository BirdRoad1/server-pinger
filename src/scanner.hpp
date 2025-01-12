#pragma once
#include "database.hpp"

void scanTask(Database *db, int ip);

void runThread(Database *db, long long startIP, long long endIP);

void runThreadVec(Database *db, std::vector<int> ips);

void scanExtraIPs(Database *db, std::vector<int> &extraIPs);

/**
 * Prints scanner progress periodically
 */
void statsThread();

void startScanner(Database *db);