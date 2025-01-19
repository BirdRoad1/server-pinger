#include "scanner.hpp"
#include "database.hpp"
#include <optional>
#include <iostream>
#include <signal.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: mcpinger <threads>" << std::endl;
        return 1;
    }

    char *threadsStr = argv[1];
    char *endptr;
    long threads = strtol(threadsStr, &endptr, 10);
    if (threadsStr == endptr || threads > 10000)
    {
        std::cout << "Invalid threads count" << std::endl;
        return 1;
    }

    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    Database db;

    if (!db.connect())
    {
        return 1;
    }

    try
    {
        db.setupDatabase();
    }
    catch (std::exception &err)
    {
        std::cout << "Failed to setup database: " << err.what() << std::endl;
    }

    startScanner(&db, threads);
    return 0;
}