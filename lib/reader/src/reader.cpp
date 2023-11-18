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
