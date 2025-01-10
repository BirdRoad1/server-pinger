#include "ping_parse_exception.hpp"

const char *ping_parse_exception::what() const noexcept
{
    return this->message;
}
