#pragma once
#include "database.hpp"

void runThread(Database *db, uint32_t startIP, uint32_t endIP);

/**
 * Prints scanner progress periodically
 */
void statsThread();

void startScanner(Database *db);