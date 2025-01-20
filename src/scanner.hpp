#pragma once
#include "database.hpp"
#include "logger.hpp"

extern Logger* logger;

void scanTask(Database *db, int ip);

void runThread(Database *db);

void scanExtraIPs(Database *db, std::vector<unsigned int> &extraIPs);

/**
 * Prints scanner progress periodically
 */
[[noreturn]] void statsThread();

void startScanner(Database *db, long numThreads);