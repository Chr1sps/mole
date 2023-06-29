#ifndef __READER_HPP__
#define __READER_HPP__
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

struct Position
{
    unsigned line, column;

    Position() : line(1), column(1)
    {
    }
};

class Reader
{
    Position current_position;

  protected:
    void update_position(wchar_t ch)
    {
        if (ch == L'\n')
        {
            ++this->current_position.line;
            this->current_position.column = 0;
        }
        ++this->current_position.column;
    }

  public:
    const Position &get_position() const noexcept
    {
        return this->current_position;
    };

    virtual wchar_t peek() = 0;
    virtual wchar_t get() = 0;
    virtual bool eof() = 0;
    virtual ~Reader() = default;
};

using ReaderPtr = std::unique_ptr<Reader>;

template <typename T>
concept DerivedFromWistream = std::derived_from<T, std::wistream>;

template <DerivedFromWistream T> class IStreamReader : public Reader
{

  protected:
    T driver;

  public:
    wchar_t peek() override
    {
        return this->driver.peek();
    }

    wchar_t get() override
    {
        auto result = this->driver.get();
        this->update_position(result);
        return result;
    }

    bool eof() override
    {
        return this->driver.eof();
    }
};

class ConsoleReader : public Reader
{
  public:
    wchar_t peek() override
    {
        return std::wcin.peek();
    }

    wchar_t get() override
    {
        auto result = std::wcin.get();
        this->update_position(result);
        return result;
    }

    bool eof() override
    {
        return std::wcin.eof();
    }
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
    StringReader(const std::wstring &code)
        : IStreamReader<std::wistringstream>()
    {
        this->driver.str(code);
    }
};
#endif