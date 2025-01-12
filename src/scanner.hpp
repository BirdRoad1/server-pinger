#pragma once
#include "database.hpp"

void runThread(Database *db, long long startIP, long long endIP);

/**
 * Prints scanner progress periodically
 */
void statsThread();

void startScanner(Database *db);