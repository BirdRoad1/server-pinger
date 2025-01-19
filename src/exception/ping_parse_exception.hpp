#pragma once
#include <stdexcept>

class ping_parse_exception : public std::exception
{
private:
    const char *message;

public:
    explicit ping_parse_exception(const char *msg)
    {
        this->message = msg;
    }

    const char *what() const noexcept override;
};