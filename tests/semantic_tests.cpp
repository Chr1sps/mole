#include "exceptions.hpp"
#include "parser.hpp"
#include "semantic_checker.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>

void check_source(const std::wstring &source)
{
    auto locale = Locale("en_US.utf8");
    auto parser = Parser(Lexer::from_wstring(source));
    auto program = parser.parse();
    auto checker = SemanticChecker();
    checker.visit(*program);
}

#define CHECK_VALID(source) REQUIRE_NOTHROW(check_source(source))
#define CHECK_INVALID(source)                                                 \
    REQUIRE_THROWS_AS(check_source(source), SemanticException)

TEST_CASE("Global constants.")
{
    CHECK_INVALID(L"let var;");
    CHECK_INVALID(L"let var: i32;");
    CHECK_VALID(L"let var = 5;");
    CHECK_VALID(L"let var: i32 = 5;");
}

TEST_CASE("Global mutables.")
{
    CHECK_INVALID(L"let mut var;");
    CHECK_VALID(L"let mut var: i32;");
    CHECK_VALID(L"let mut var = 5;");
    CHECK_VALID(L"let mut var: i32 = 5;");
}

TEST_CASE("Global constants, invalid - redefinitions.")
{
    CHECK_INVALID(L"let var = 5; let var = 7;");
}

TEST_CASE("Global constants, invalid - name 'main'.")
{
    CHECK_INVALID(L"let main = 5;");
}

TEST_CASE("Empty main - valid.")
{
    CHECK_VALID(L"fn main(){}");
    CHECK_VALID(L"fn main()=>!{}");
}

TEST_CASE("Empty main - invalid return type signature.")
{
    CHECK_INVALID(L"fn main()=>i32{}");
}

TEST_CASE("Empty main - no return value.")
{
    CHECK_INVALID(L"fn main()=>u8{}");
}

TEST_CASE("Valid main, with u8 return.")
{
    CHECK_VALID(L"fn main()=>u8{return 8;}");
}

TEST_CASE("Main with statements, without needed return.")
{
    CHECK_INVALID(L"fn main()=>u8{let mut var : i32;}");
    CHECK_INVALID(L"fn main()=>u8{let mut var : i32;let mut foo : i32;}");
}