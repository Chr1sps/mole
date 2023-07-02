#include "reader.hpp"

void Reader::update_position(const wchar_t &ch)
{
    if (ch == L'\n')
    {
        ++this->current_position.line;
        this->current_position.column = 0;
    }
    ++this->current_position.column;
}

bool operator==(const Position &first, const Position &other)
{
    return first.line == other.line && first.column == other.column;
}

std::ostream &operator<<(std::ostream &os, const Position &pos)
{
    return (os << "{line: " << pos.line << ", col: " << pos.column << "}");
}
