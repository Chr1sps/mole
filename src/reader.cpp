#include "reader.hpp"
#include <iostream>

wchar_t ConsoleReader::peek()
{
    return std::cin.peek();
}

wchar_t ConsoleReader::get()
{
    return std::cin.get();
}

wchar_t StringReader::peek()
{
    return this->code.peek();
}

wchar_t StringReader::get()
{
    return this->code.get();
}

wchar_t FileReader::peek()
{
    return this->driver.peek();
}

wchar_t FileReader::get()
{
    return this->driver.get();
}