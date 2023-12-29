#include "position.hpp"
#include <ostream>

bool operator==(const Position &first, const Position &other) noexcept
{
    return first.line == other.line && first.column == other.column;
}

std::ostream &operator<<(std::ostream &os, const Position &pos) noexcept
{
    return (os << "{line: " << pos.line << ", col: " << pos.column << "}");
}