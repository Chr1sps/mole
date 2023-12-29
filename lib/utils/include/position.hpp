#ifndef __POSITION_HPP__
#define __POSITION_HPP__
#include <ostream>

struct Position
{
    unsigned line, column;

    constexpr Position(const unsigned &line, const unsigned &column) noexcept
        : line(line), column(column)
    {
    }
};

inline constexpr bool operator==(const Position &first,
                                 const Position &other) noexcept
{
    return first.line == other.line && first.column == other.column;
}

// for debugging purposes
std::ostream &operator<<(std::ostream &os, const Position &pos) noexcept;

#endif