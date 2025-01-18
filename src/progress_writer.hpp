#pragma once
#include <string>
#include <fstream>
#include <mutex>

class ProgressWriter
{
private:
    static std::mutex mutex;
    static std::ofstream *stream;
    static bool isOpen;
    static void open();

public:
    static void writeIP(std::string str);
    static void close();
};