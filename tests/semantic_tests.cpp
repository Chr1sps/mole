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
#define CHECK_EXCEPTION(source, exception)                                    \
    REQUIRE_THROWS_AS(check_source(source), exception)

TEST_CASE("Empty main.")
{
    CHECK_VALID(L"fn main(){}");
    CHECK_VALID(L"fn main()=>u8{}");
}