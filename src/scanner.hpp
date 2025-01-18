#pragma once
#include "database.hpp"

void scanTask(Database *db, int ip);

void runThread(Database *db);

void scanExtraIPs(Database *db, std::vector<unsigned int> &extraIPs);

/**
 * Prints scanner progress periodically
 */
void statsThread();

void startScanner(Database *db);