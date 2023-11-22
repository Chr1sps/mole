#include "reader.hpp"

bool operator==(const IndexedChar &first, const wchar_t &other)
{
    return first.character == other;
}

bool operator!=(const IndexedChar &first, const wchar_t &other)
{
    return first.character != other;
}

std::optional<IndexedChar> Reader::peek()
{
    auto result_char = this->peek_raw();
    if (static_cast<std::wint_t>(result_char) == WEOF)
        return std::nullopt;
    return IndexedChar(result_char, this->current_position);
}

void Reader::update_position(const wchar_t &ch)
{
    if (ch == L'\n')
    {
        ++this->current_position.line;
        this->current_position.column = 0;
    }
    ++this->current_position.column;
}

std::optional<IndexedChar> Reader::get()
{
    auto result_char = this->get_raw();
    if (this->eof())
        return std::nullopt;

    // converting Windows newline to Unix newline
    if (result_char == '\r' && this->peek_raw() == L'\n')
    {
        result_char = '\n';
        this->get_raw();
    }

    auto result = IndexedChar(result_char, this->current_position);
    this->update_position(result_char);
    return result;
}

FileReader::FileReader(const std::filesystem::path &path,
                       const std::locale &locale)
    : IStreamReader<std::wifstream>(locale), file_path(path)
{
    this->driver.open(path);
    this->driver.imbue(locale);
    if (!this->driver.good())
        throw std::ios_base::failure(
            build_string("File not found: ", this->file_path));
}