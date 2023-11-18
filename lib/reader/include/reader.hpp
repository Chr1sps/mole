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

class Reader
{
    Position current_position;

  protected:
    std::locale locale;

    void update_position(const wchar_t &ch);
    virtual wchar_t get_raw() = 0;
    virtual wchar_t peek_raw() = 0;

    Reader(const std::locale &locale) : locale(locale)
    {
    }

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

    FileReader(const std::filesystem::path &path, const std::locale &locale)
        : IStreamReader<std::wifstream>(locale), file_path(path)
    {
        this->driver.open(path);
        this->driver.imbue(locale);
        if (!this->driver.good())
            throw std::ios_base::failure(
                build_string("File not found: ", this->file_path));
    }

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