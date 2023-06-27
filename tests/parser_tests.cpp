#include "debug_visitor.hpp"
#include "parser.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>

bool compare_output(Parser &&parser, const std::wstring &expected)
{
    auto out_stream = std::wostringstream();
    auto visitor = DebugVisitor(out_stream);
    visitor.visit(*(parser.parse()));
    auto result = out_stream.str();
    return result == expected;
}

#define COMPARE(source, output)                                               \
    REQUIRE(compare_output(Parser(Lexer::from_wstring(source)), output))
#define REPR_CHECK(source)                                                    \
    REQUIRE(compare_output(Parser(Lexer::from_wstring(source)), source))

TEST_CASE("Empty code.")
{
    REPR_CHECK(L"");
}

TEST_CASE("Variables.", "[VARS]")
{
    SECTION("No type, no value;")
    {
        REPR_CHECK(L"let var;");
    }
    SECTION("Type.")
    {
        REPR_CHECK(L"let var:i32;");
        REPR_CHECK(L"let var:f64;");
        COMPARE(L"let var:fn();", L"let var:fn()=>!;");
        COMPARE(L"let var:fn(i32);", L"let var:fn(i32)=>!;");
        REPR_CHECK(L"let var:fn(i32)=>i32;");
        REPR_CHECK(L"let var:fn(i32,f64)=>i32;");
    }
    SECTION("Value - constants.")
    {
        REPR_CHECK(L"let var=5;");
        COMPARE(L"let var=5.25;", L"let var=5.25d;");
        REPR_CHECK(L"let var=5.25d;");
        REPR_CHECK(L"let var=5.25f;");
    }
}

TEST_CASE("Binary expressions.", "[VARS], [BINOP]")
{
    COMPARE(L"let var=5+5;", L"let var=(5+5);");
    COMPARE(L"let var=5.5d+5;", L"let var=(5.5d+5);");
    COMPARE(L"let var=5.5f+5;", L"let var=(5.5f+5);");
    COMPARE(L"let var=5.5f>>5;", L"let var=(5.5f>>5);");
}

TEST_CASE("Function definitions.", "[FUNC]")
{
    SECTION("No args, no return type.")
    {
        COMPARE(L"fn foo(){}", L"fn foo()=>!{}");
    }
    SECTION("Return type.")
    {
        REPR_CHECK(L"fn foo()=>i32{}");
        COMPARE(L"fn foo() => fn() {}", L"fn foo()=>fn()=>!{}");
    }
    SECTION("Args.")
    {
        COMPARE(L"fn foo(x: i32, y: f64) {}", L"fn foo(x:i32,y:f64)=>!{}");
    }
    SECTION("Combined.")
    {
        REPR_CHECK(L"fn foo(x:i32,y:f64)=>i32{}");
    }
}

TEST_CASE("Scopes in functions.", "[SCOPE]")
{
    COMPARE(L"fn foo(){{}}", L"fn foo()=>!{{}}");
}

TEST_CASE("Externs.", "[EXT]")
{
    COMPARE(L"extern foo();", L"ext foo()=>!;");
}