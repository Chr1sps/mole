#include "reader.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace std;

std::vector<unsigned> line_lengths(const std::wstring &input)
{
    std::vector<unsigned> result;
    std::wistringstream istream(input);
    std::wstring line;

    while (std::getline(istream, line))
        result.push_back(line.size() + 1);

    return result;
}

void get_and_check(Reader &reader, const Position &pos,
                   const std::optional<wchar_t> &value, const bool eof = false)
{
    auto result = reader.get();
    REQUIRE(reader.get_position() == pos);
    REQUIRE(result == value);
    REQUIRE(reader.eof() == eof);
}

void test_reader_sequence(std::wstring &&str)
{
    std::optional<wchar_t> result;
    auto reader = StringReader(str);
    auto lengths = line_lengths(str);

    for (unsigned i = 0, line = 0, col = 1;
         i < str.size() && (col != lengths[line] || line != lengths.size());
         ++i)
    {
        REQUIRE(reader.get_position() == Position(line + 1, col));
        if (col == lengths[line] && line != lengths.size() - 1)
        {
            ++line;
            col = 0;
        }
        ++col;
        get_and_check(reader, Position(line + 1, col), {str[i]});
    }
    result = reader.get();
    if (lengths.size())
    {
        REQUIRE(reader.get_position() ==
                Position(lengths.size(), lengths[lengths.size() - 1]));
    }
    else
    {
        REQUIRE(reader.get_position() == Position(1u, 1u));
    }
    REQUIRE_FALSE(result.has_value());
    REQUIRE(reader.eof());
}

TEST_CASE("Empty source.")
{
    test_reader_sequence(L"");
}

TEST_CASE("One line.")
{
    test_reader_sequence(L"one two three four");
}

TEST_CASE("Multiple lines.")
{
    test_reader_sequence(L"one\ntwo\nthree\nfour");
}