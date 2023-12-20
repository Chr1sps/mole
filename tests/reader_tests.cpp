#include "reader.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace std;

void get_and_check(Reader &reader, const wchar_t &value,
                   const Position &position)
{
    auto [chr, pos] = reader.get();
    REQUIRE(chr.has_value());
    REQUIRE(chr == value);
    REQUIRE(pos == position);
}

void get_and_check_eof(Reader &reader)
{
    auto [chr, _] = reader.get();
    REQUIRE(!chr.has_value());
}

TEST_CASE("Empty source.")
{
    auto reader = StringReader(L"");
    get_and_check_eof(reader);
    SECTION("Calling get() after EOF.")
    {
        get_and_check_eof(reader);
    }
}

TEST_CASE("Single line.")
{
    auto reader = StringReader(L"A");
    get_and_check(reader, L'A', Position(1, 1));
    get_and_check_eof(reader);
}

TEST_CASE("Two lines - newline handling.")
{
    SECTION("Unix newline.")
    {
        auto reader = StringReader(L"A\nB");
        get_and_check(reader, L'A', Position(1, 1));
        get_and_check(reader, L'\n', Position(1, 2));
        get_and_check(reader, L'B', Position(2, 1));
        get_and_check_eof(reader);
    }
    SECTION("Windows newline.")
    {
        auto reader = StringReader(L"A\r\nB");
        get_and_check(reader, L'A', Position(1, 1));
        get_and_check(reader, L'\n', Position(1, 2));
        get_and_check(reader, L'B', Position(2, 1));
        get_and_check_eof(reader);
    }
    SECTION("Just \\r.")
    {
        auto reader = StringReader(L"A\rB");
        get_and_check(reader, L'A', Position(1, 1));
        get_and_check(reader, L'\r', Position(1, 2));
        get_and_check(reader, L'B', Position(1, 3));
        get_and_check_eof(reader);
    }
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