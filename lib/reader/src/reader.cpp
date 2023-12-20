#include "reader.hpp"

void Reader::update_position(const wchar_t &ch)
{
    if (ch == '\n')
    {
        ++this->current_position.line;
        this->current_position.column = 0;
    }
    ++this->current_position.column;
}

CharWithPos Reader::get()
{
    auto result_char = this->get_raw();
    if (static_cast<std::wint_t>(result_char) == WEOF)
        return {std::nullopt, this->current_position};

    // converting Windows newline to Unix newline
    if (result_char == '\r' && this->peek_raw() == L'\n')
    {
        result_char = '\n';
        this->get_raw();
    }

    auto result =
        std::tuple(std::make_optional(result_char), this->current_position);
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