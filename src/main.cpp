#include "scanner.hpp"
#include "database.hpp"
#include <optional>
#include <iostream>
#include <csignal>
#include "logger.hpp"

Logger *logger = Logger::getLogger();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        logger->error()->write("Usage: mcpinger <threads>")->end();
        return 1;
    }

    char *threadsStr = argv[1];
    char *endptr;
    long threads = strtol(threadsStr, &endptr, 10);
    if (threadsStr == endptr || threads > 10000) {
        logger->error()->write("Invalid threads count")->end();
        return 1;
    }

    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    Database db;

    if (!db.connect()) {
        return 1;
    }

    try {
        db.setupDatabase();
    } catch (std::exception &err) {
        logger->error()->write("Failed to setup database: ")->write(err.what())->end();
    }

    startScanner(&db, threads);

    Logger::cleanup();
    logger = nullptr;

    return 0;
}
