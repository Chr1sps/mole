#include "exceptions.hpp"
#include "parser.hpp"
#include "print_visitor.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>

bool compare_output(Parser &&parser, const std::wstring &expected)
{
    auto out_stream = std::wostringstream();
    auto visitor = PrintVisitor(out_stream, true);
    visitor.visit(*(parser.parse()));
    auto result = out_stream.str();
    return result == expected;
}

void parse_source(const std::wstring &source)
{
    auto parser = Parser(Lexer::from_wstring(source));
    parser.parse();
}

#define COMPARE(source, output)                                               \
    REQUIRE(compare_output(Parser(Lexer::from_wstring(source)), output))
#define REPR_CHECK(source)                                                    \
    REQUIRE(compare_output(Parser(Lexer::from_wstring(source)), source))
#define CHECK_EXCEPTION(source, exception)                                    \
    REQUIRE_THROWS_AS(parse_source(source), exception)
#define FN_WRAP(source) L"fn foo()=>i32{" + std::wstring(source) + L"}"
#define COMPARE_STATEMENT(source, output)                                     \
    REQUIRE(compare_output(Parser(Lexer::from_wstring(FN_WRAP(source))),      \
                           FN_WRAP(output)))
#define REPR_STATEMENT(source) COMPARE_STATEMENT(source, source)

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
    SECTION("Value - literals.")
    {
        REPR_CHECK(L"let var=5;");
        COMPARE(L"let var=5.25;", L"let var=5.25d;");
        REPR_CHECK(L"let var=5.25d;");
        REPR_CHECK(L"let var=5.25f;");
    }
    SECTION("Type and value.")
    {
        REPR_CHECK(L"let var:i32=5;");
        COMPARE(L"let var:f64=5.25;", L"let var:f64=5.25d;");
        REPR_CHECK(L"let var:f64=5.25d;");
        REPR_CHECK(L"let var:f32=5.25f;");
    }
    SECTION("Mutables.")
    {
        REPR_CHECK(L"let mut var=32;");
        REPR_CHECK(L"let mut var:i32=32;");
    }
}

TEST_CASE("Binary operators.", "[VARS], [BINOP]")
{
    COMPARE(L"let var=5+5;", L"let var=(5+5);");
    COMPARE(L"let var=5.5d+5;", L"let var=(5.5d+5);");
    COMPARE(L"let var=5.5f+5;", L"let var=(5.5f+5);");
    COMPARE(L"let var=5.5f>>5;", L"let var=(5.5f>>5);");
}

TEST_CASE("Unary operators.", "[VARS], [UNOP]")
{
    COMPARE(L"let var=++5;", L"let var=(++5);");
    COMPARE(L"let var=--5;", L"let var=(--5);");
    COMPARE(L"let var=!5;", L"let var=(!5);");
    COMPARE(L"let var=~5;", L"let var=(~5);");
}

TEST_CASE("Nested operators - only binary.")
{
    COMPARE(L"let var=2*2*2;", L"let var=((2*2)*2);");
    COMPARE(L"let var=2+2*2;", L"let var=(2+(2*2));");
    COMPARE(L"let var=2*2/2%2;", L"let var=(((2*2)/2)%2);");
}

TEST_CASE("Nested operators - only unary.")
{
    COMPARE(L"let var=--++!~2;", L"let var=(--(++(!(~2))));");
}

TEST_CASE("Nested operators - mixed.")
{
    COMPARE(L"let var=--2+--2;", L"let var=((--2)+(--2));");
    COMPARE(L"let var=++1*--2+!3*~4-5>>6;",
            L"let var=(((((++1)*(--2))+((!3)*(~4)))-5)>>6);");
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
    SECTION("Const function.")
    {
        REPR_CHECK(L"fn const foo(x:i32,y:f64)=>i32{}");
    }
}

TEST_CASE("In-place lambdas.")
{
    SECTION("Correct.")
    {
        REPR_CHECK(L"let foo=boo(1,2,3,4);");
        REPR_CHECK(L"let foo=boo(1,_,3,_);");
        REPR_CHECK(L"let foo=boo(1,2,...);");
        REPR_CHECK(L"let foo=boo(1,_,...);");
    }
    SECTION("Exceptions.")
    {
        CHECK_EXCEPTION(L"let foo=boo(_,...,_);", ParserException);
        CHECK_EXCEPTION(L"let foo=boo(1,...,4);", ParserException);
        CHECK_EXCEPTION(L"let foo=boo(...,3,4);", ParserException);
    }
}

TEST_CASE("Scopes in functions.", "[SCOPE]")
{
    COMPARE(L"fn foo(){{}}", L"fn foo()=>!{{}}");
}

TEST_CASE("Externs.", "[EXT]")
{
    COMPARE(L"extern foo();", L"extern foo()=>!;");
    COMPARE(L"extern foo(x:i32);", L"extern foo(x:i32)=>!;");
}

TEST_CASE("Assign statements.")
{
    REPR_STATEMENT(L"x=5;");
    REPR_STATEMENT(L"x+=5;");
    REPR_STATEMENT(L"x-=5;");
    REPR_STATEMENT(L"x*=5;");
    REPR_STATEMENT(L"x/=5;");
    REPR_STATEMENT(L"x%=5;");
    REPR_STATEMENT(L"x^^=5;");
}