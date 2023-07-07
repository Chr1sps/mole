#ifndef __READER_HPP__
#define __READER_HPP__
#include "locale.hpp"
#include <concepts>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

struct Position
{
    unsigned line, column;

    Position(const unsigned &line, const unsigned &column)
        : line(line), column(column)
    {
    }

    Position() : Position(1, 1)
    {
    }

    friend bool operator==(const Position &first, const Position &other);
};

// for debugging purposes

std::ostream &operator<<(std::ostream &os, const Position &pos);

class Reader
{
    Position current_position;

  protected:
    void update_position(const wchar_t &ch);
    virtual wchar_t get_raw() = 0;
    virtual wchar_t peek_raw() = 0;

  public:
    const Position &get_position() const noexcept
    {
        return this->current_position;
    };

    std::optional<wchar_t> peek()
    {
        auto result = this->peek_raw();
        if (static_cast<std::wint_t>(result) == WEOF)
            return std::nullopt;
        return result;
    }

    std::optional<wchar_t> get()
    {
        auto result = this->get_raw();
        if (this->eof())
            return std::nullopt;
        else
        {
            this->update_position(result);
            return result;
        }
    }

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

    wchar_t get_raw() override
    {
        return this->driver.get();
    }

    wchar_t peek_raw() override
    {
        return this->driver.peek();
    }

  public:
    bool eof() override
    {
        return this->driver.eof();
    }
};

class ConsoleReader : public Reader
{
  protected:
    wchar_t get_raw() override
    {
        return std::wcin.get();
    }

    wchar_t peek_raw() override
    {
        return std::wcin.peek();
    }

  public:
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
        this->driver.imbue(Locale::get().locale());
        if (!this->driver.good())
            throw std::exception();
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