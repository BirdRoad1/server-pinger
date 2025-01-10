#include "net_exception.hpp"

const char *net_exception::what() const noexcept
{
    return this->message;
}
