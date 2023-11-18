#include "position.hpp"

bool operator==(const Position &first, const Position &other)
{
    return first.line == other.line && first.column == other.column;
}

std::ostream &operator<<(std::ostream &os, const Position &pos)
{
    return (os << "{line: " << pos.line << ", col: " << pos.column << "}");
}