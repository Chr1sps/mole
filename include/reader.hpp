#ifndef __READER_HPP__
#define __READER_HPP__
#include <memory>
#include <fstream>
#include <sstream>
class Reader
{
public:
    virtual wchar_t peek() = 0;
    virtual wchar_t get() = 0;
    virtual ~Reader() = default;
};
using ReaderPtr = std::unique_ptr<Reader>;

class ConsoleReader : public Reader
{
public:
    ConsoleReader() = default;
    wchar_t peek() override;
    wchar_t get() override;
};

class FileReader : public Reader
{
    std::wifstream driver;

public:
    FileReader(const std::string &file_name) : driver(file_name) {}
    wchar_t peek() override;
    wchar_t get() override;
};

class StringReader : public Reader
{
    std::wstringstream code;

public:
    StringReader(const std::wstring &code) : code(code) {}
    wchar_t peek() override;
    wchar_t get() override;
};
#endif