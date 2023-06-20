#include "reader.hpp"
#include <iostream>

char ConsoleReader::peek()
{
    return std::cin.peek();
}

char ConsoleReader::get()
{
    return std::cin.get();
}

char StringReader::get()
{
    return this->code.get();
}

char StringReader::peek()
{
    return this->code.peek();
}