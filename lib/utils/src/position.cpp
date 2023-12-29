#include "position.hpp"
#include <ostream>

std::ostream &operator<<(std::ostream &os, const Position &pos) noexcept
{
    return (os << "{line: " << pos.line << ", col: " << pos.column << "}");
}