#ifndef __READER_HPP__
#define __READER_HPP__
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
class Reader
{
public:
    virtual wchar_t peek() = 0;
    virtual wchar_t get() = 0;
    virtual bool eof() = 0;
    virtual ~Reader() = default;
};
using ReaderPtr = std::unique_ptr<Reader>;

template <typename T>
concept DerivedFromWistream = std::derived_from<T, std::wistream>;

template <typename T>
class IStreamReader : public Reader
{
    static_assert(DerivedFromWistream<T>,
                  "T must be derived from the std::wistream class.");

protected:
    T driver;

public:
    wchar_t peek() override { return this->driver.peek(); }
    wchar_t get() override { return this->driver.get(); }
    bool eof() override { return this->driver.eof(); }
};

class ConsoleReader : public Reader
{
public:
    wchar_t peek() override { return std::wcin.peek(); }
    wchar_t get() override { return std::wcin.get(); }
    bool eof() override { return std::wcin.eof(); }
};

class FileReader : public IStreamReader<std::wifstream>
{
public:
    FileReader(const std::string &file_name) : IStreamReader<std::wifstream>()
    {
        this->driver.open(file_name);
    }
};

class StringReader : public IStreamReader<std::wistringstream>
{
public:
    StringReader(const std::wstring &code) : IStreamReader<std::wistringstream>()
    {
        this->driver.str(code);
    }
};
#endif