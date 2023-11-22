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

void peek_and_check(Reader &reader, const wchar_t &value, const Position &pos)
{
    auto result = reader.peek();
    REQUIRE(result.has_value());
    REQUIRE(result->character == value);
    REQUIRE(result->position == pos);
}

void check_peek_and_get(Reader &reader, const wchar_t &value,
                        const Position &pos)
{
    peek_and_check(reader, value, pos);
    get_and_check(reader, value, pos);
}

void get_and_check_eof(Reader &reader)
{
    auto result = reader.get();
    REQUIRE(!result.has_value());
}

void peek_and_check_eof(Reader &reader)
{
    auto result = reader.peek();
    REQUIRE(!result.has_value());
}

void check_peek_and_get_eof(Reader &reader)
{
    peek_and_check_eof(reader);
    get_and_check_eof(reader);
}

TEST_CASE("Empty source.")
{
    auto reader = StringReader(L"");
    check_peek_and_get_eof(reader);
    SECTION("Calling get() after EOF.")
    {
        check_peek_and_get_eof(reader);
    }
}

TEST_CASE("Single line.")
{
    auto reader = StringReader(L"A");
    check_peek_and_get(reader, L'A', Position(1, 1));
    check_peek_and_get_eof(reader);
}

TEST_CASE("Two lines - newline handling.")
{
    SECTION("Unix newline.")
    {
        auto reader = StringReader(L"A\nB");
        check_peek_and_get(reader, L'A', Position(1, 1));
        check_peek_and_get(reader, L'\n', Position(1, 2));
        check_peek_and_get(reader, L'B', Position(2, 1));
        check_peek_and_get_eof(reader);
    }
    SECTION("Windows newline.")
    {
        auto reader = StringReader(L"A\r\nB");
        check_peek_and_get(reader, L'A', Position(1, 1));
        check_peek_and_get(reader, L'\n', Position(1, 2));
        check_peek_and_get(reader, L'B', Position(2, 1));
        check_peek_and_get_eof(reader);
    }
    SECTION("Just \\r.")
    {
        auto reader = StringReader(L"A\rB");
        check_peek_and_get(reader, L'A', Position(1, 1));
        check_peek_and_get(reader, L'\r', Position(1, 2));
        check_peek_and_get(reader, L'B', Position(1, 3));
        check_peek_and_get_eof(reader);
    }
}

TEST_CASE("UTF-8")
{
    auto reader = StringReader(L"ąęó😊ł");
    check_peek_and_get(reader, L'ą', Position(1, 1));
    check_peek_and_get(reader, L'ę', Position(1, 2));
    check_peek_and_get(reader, L'ó', Position(1, 3));
    check_peek_and_get(reader, L'😊', Position(1, 4));
    check_peek_and_get(reader, L'ł', Position(1, 5));
    check_peek_and_get_eof(reader);
}