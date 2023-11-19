#ifndef __READER_HPP__
#define __READER_HPP__
#include "locale.hpp"
#include "position.hpp"
#include "string_builder.hpp"
#include <concepts>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

struct IndexedChar
{
    wchar_t character;
    Position position;

    IndexedChar(const wchar_t character, const Position &position)
        : character(character), position(position)
    {
    }

    friend bool operator==(const IndexedChar &first, const wchar_t &other);
    friend bool operator!=(const IndexedChar &first, const wchar_t &other);
};

class Reader
{
    Position current_position;

  protected:
    std::locale locale;

    void update_position(const wchar_t &ch);

    virtual wchar_t get_raw() = 0;
    virtual wchar_t peek_raw() = 0;

    Reader(const std::locale &locale) : current_position(1, 1), locale(locale)
    {
    }

  public:
    std::optional<IndexedChar> peek();
    std::optional<IndexedChar> get();

    const std::locale &get_locale() const
    {
        return this->locale;
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

    IStreamReader(const std::locale &locale) : Reader(locale)
    {
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
    ConsoleReader(const ConsoleReader &) = delete;

    ConsoleReader(const std::locale &locale) : Reader(locale)
    {
    }

    ConsoleReader() : ConsoleReader(std::locale())
    {
    }

    bool eof() override
    {
        return std::wcin.eof();
    }
};

class FileReader : public IStreamReader<std::wifstream>
{
    std::filesystem::path file_path;

  public:
    FileReader(const FileReader &) = delete;

    FileReader(const std::filesystem::path &path, const std::locale &locale);

    FileReader(const std::string &file_name)
        : FileReader(file_name, std::locale())
    {
    }

    virtual ~FileReader()
    {
        this->driver.close();
    }
};

class StringReader : public IStreamReader<std::wistringstream>
{
  public:
    StringReader(const StringReader &) = delete;

    StringReader(const std::wstring &code, const std::locale &locale)
        : IStreamReader<std::wistringstream>(locale)
    {
        this->driver.str(code);
    }

    StringReader(const std::wstring &code)
        : IStreamReader<std::wistringstream>(std::locale())
    {
        this->driver.str(code);
    }
};
#endif