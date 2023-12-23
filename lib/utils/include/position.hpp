#ifndef __POSITION_HPP__
#define __POSITION_HPP__
#include <ostream>

struct Position
{
    unsigned line, column;

    Position(const unsigned &line, const unsigned &column) noexcept
        : line(line), column(column)
    {
    }

    // Position(const Position &) = default;
    // Position(Position &&) = default;
    // Position &operator=(const Position &other) = default;

    friend bool operator==(const Position &first,
                           const Position &other) noexcept;
};

// for debugging purposes

std::ostream &operator<<(std::ostream &os, const Position &pos) noexcept;

#endif