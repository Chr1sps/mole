#ifndef __READER_HPP__
#define __READER_HPP__
#include <memory>
#include <fstream>
#include <sstream>
class Reader
{
public:
    virtual char peek() = 0;
    virtual char get() = 0;
    virtual ~Reader() = default;
};
using ReaderPtr = std::unique_ptr<Reader>;
class ConsoleReader : public Reader
{
public:
    ConsoleReader() = default;
    char peek() override;
    char get() override;
};
class FileReader : public Reader
{
    std::ifstream driver;

public:
    FileReader(const std::string &file_name) : driver(file_name) {}
};
class StringReader : public Reader
{
    std::stringstream code;

public:
    StringReader(const std::string &code) : code(code) {}
    char peek() override;
    char get() override;
};
#endif