#pragma once
#include <stdexcept>

class PingParseException : public std::exception
{
private:
    const char *message;

public:
    PingParseException(const char *msg)
    {
        this->message = msg;
    }

    const char *what() const noexcept override
    {
        return this->message;
    }
};