#pragma once
#include <iomanip>
#include <iostream>
#include <ostream>
#include <bits/stl_deque.h>

class Logger {
private:
    static Logger *logger;

    static void startLine(const std::string &level);

public:
    static Logger *getLogger();

    Logger *info();

    Logger *error();

    Logger *warn();

    Logger *write(const std::string &value);

    Logger *write(int value);

    Logger *write(double value);

    Logger *write(long long value);

    Logger *write(std::size_t value);

    Logger *fixed();

    Logger *setPrecision(int precision);

    Logger *end();

    static void cleanup() ;
};
