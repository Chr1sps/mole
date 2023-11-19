#include "reader.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace std;

void get_and_check(Reader &reader, const wchar_t &value, const Position &pos)
{
    auto result = reader.get();
    REQUIRE(result.has_value());
    REQUIRE(result->character == value);
    REQUIRE(result->position == pos);
}

void get_and_check_eof(Reader &reader)
{
    auto result = reader.get();
    REQUIRE(!result.has_value());
}

TEST_CASE("Empty source.")
{
    auto reader = StringReader(L"");
    get_and_check_eof(reader);
}

TEST_CASE("Single line.")
{
    auto reader = StringReader(L"A");
    get_and_check(reader, L'A', Position(1, 1));
    get_and_check_eof(reader);
}

TEST_CASE("Two lines.")
{
    auto reader = StringReader(L"A\nB");
    get_and_check(reader, L'A', Position(1, 1));
    get_and_check(reader, L'\n', Position(1, 2));
    get_and_check(reader, L'B', Position(2, 1));
    get_and_check_eof(reader);
}

TEST_CASE("UTF-8")
{
    auto reader = StringReader(L"ąęó😊ł");
    get_and_check(reader, L'ą', Position(1, 1));
    get_and_check(reader, L'ę', Position(1, 2));
    get_and_check(reader, L'ó', Position(1, 3));
    get_and_check(reader, L'😊', Position(1, 4));
    get_and_check(reader, L'ł', Position(1, 5));
    get_and_check_eof(reader);
}