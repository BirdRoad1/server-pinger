#pragma once
#include <stdexcept>

class net_exception : public std::exception
{
private:
    const char *message;

public:
    net_exception(const char *message)
    {
        this->message = message;
    }

    net_exception() : net_exception("Network error while making request") {}

    const char *what() const noexcept override;
};