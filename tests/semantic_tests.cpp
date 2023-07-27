#include "exceptions.hpp"
#include "parser.hpp"
#include "semantic_checker.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>

void check_source(const std::wstring &source)
{
    auto parser = Parser(Lexer::from_wstring(source));
    auto program = parser.parse();
    auto checker = SemanticChecker();
    checker.visit(*program);
}

#define CHECK_VALID(source) REQUIRE_NOTHROW(check_source(source))
#define CHECK_INVALID(source, exception)                                      \
    REQUIRE_THROWS_AS(check_source(source), exception)

TEST_CASE("Empty main - valid.")
{
    CHECK_VALID(L"fn main(){}");
    CHECK_VALID(L"fn main()=>!{}");
}

TEST_CASE("Empty main - invalid return type signature.")
{
    CHECK_INVALID(L"fn main()=>i32{}", SemanticException);
}

TEST_CASE("Empty main - no return value.")
{
    CHECK_INVALID(L"fn main()=>u8{}", SemanticException);
}

TEST_CASE("Empty main - valid, with u8 return.")
{
    CHECK_VALID(L"fn main()=>u8{return 8;}");
    CHECK_VALID(L"fn main()=>u8{return 8;}");
}