#include "logger.hpp"

Logger *Logger::logger = nullptr;

void Logger::startLine(const std::string &level) {
    const time_t t1 = time(nullptr);
    std::cout << "[" << level << "] [" << std::put_time(std::localtime(&t1), "%H:%M:%S") << "] ";
}

Logger *Logger::getLogger() {
    if (logger == nullptr) {
        logger = new Logger();
    }

    return logger;
}

Logger *Logger::info() {
    startLine("INFO");
    return this;
}

Logger *Logger::error() {
    startLine("ERROR");
    return this;
}

Logger *Logger::warn() {
    startLine("WARN");
    return this;
}

Logger *Logger::write(const std::string &value) {
    std::cout << value;
    return this;
}

Logger *Logger::write(const int value) {
    std::cout << value;
    return this;
}

Logger *Logger::write(const double value) {
    std::cout << value;
    return this;
}

Logger *Logger::write(const long long value) {
    std::cout << value;
    return this;
}

Logger *Logger::write(const std::size_t value) {
    std::cout << value;
    return this;
}


Logger *Logger::fixed() {
    std::cout << std::fixed;
    return this;
}

Logger *Logger::setPrecision(const int precision) {
    std::cout << std::setprecision(precision);
    return this;
}


Logger *Logger::end() {
    std::cout << std::endl;
    return this;
}

void Logger::cleanup() {
    delete logger;
}
