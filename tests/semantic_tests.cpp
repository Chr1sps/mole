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
#define FN_WRAP(source) L"fn foo(){" + std::wstring(source) + L"}"

TEST_CASE("Variable has no value or type assigned.")
{
    CHECK_INVALID(L"let var;");
    CHECK_INVALID(L"let mut var;");
}

TEST_CASE("Variable's declared type and assigned value's type don't match")
{
    CHECK_VALID(L"let var: i32 = 1;");
    CHECK_VALID(L"let var: f64 = 0.1;");

    CHECK_INVALID(L"let var: i32 = 0.1;");
    CHECK_INVALID(L"let var: f64 = 1;");
}

TEST_CASE("Constant has no value assigned.")
{
    CHECK_INVALID(L"let var: i32;");
    CHECK_VALID(L"let var: i32 = 32;");
}

TEST_CASE("Constant cannot be reassigned.")
{
    CHECK_INVALID(FN_WRAP(L"let value = 5; value = 4;"));
}

TEST_CASE("Using an uninitialized mutable in an expression.")
{
    CHECK_INVALID(
        FN_WRAP(L"let mut value: i32; let new_value = (4 + value);"));
}

TEST_CASE("Referenced value/function is not in scope.")
{
    CHECK_INVALID(
        L"fn foo(){let value = 5;} fn goo(){let new_value = value;}");
}

TEST_CASE("Function's argument type is mismatched.")
{
    CHECK_INVALID(L"fn foo(x: i32){}"
                  L"fn boo(){let value = 0.1; let falue = foo(value);}");
    CHECK_INVALID(L"fn foo(x: i32){}"
                  L"fn boo(){foo(0.1);}");
}

TEST_CASE("Function is called with a wrong amount of arguments.")
{
    CHECK_INVALID(L"fn foo(x: i32){}"
                  L"fn boo(){let goo = foo(1,2);}");
    CHECK_INVALID(L"fn foo(x: i32){}"
                  L"fn boo(){let goo = foo();}");
}

TEST_CASE("Referenced value is not a callable.")
{
    CHECK_INVALID(FN_WRAP(L"let foo = 5; let boo = foo();"));
}

TEST_CASE("Function doesn't return when its type indicates otherwise.")
{
    CHECK_INVALID(L"fn foo(x: i32)=>i32{}");
    CHECK_VALID(L"fn foo(x: i32)=>!{}");
}

TEST_CASE("Function returns the wrong type.")
{
    CHECK_INVALID(L"fn foo()=>i32{return 0.1;}");
    CHECK_INVALID(L"fn foo()=>i32{let value = 0.1; return value;}");
}

TEST_CASE("Return statement with no value in a function that does return.")
{
    CHECK_INVALID(L"fn foo(x: i32)=>i32{return;}");
}

TEST_CASE("Function returns a value when it shouldn't.")
{
    CHECK_INVALID(L"fn foo(x: i32){return 5;}");
    CHECK_INVALID(L"fn foo(x: i32)=>!{return 5;}");
}

TEST_CASE("Variable cannot be called 'main'.")
{
    CHECK_INVALID(L"let main: i32;");
    CHECK_INVALID(L"let main: fn()=>!;");
    CHECK_INVALID(L"let main: fn()=>u8;");
}

TEST_CASE("Main should be of type fn()=>u8 or fn()=>!.")
{
    CHECK_INVALID(L"fn main()=>i32{return 3;}");
    CHECK_INVALID(L"fn main()=>f64{return 0.1;}");
}

TEST_CASE("Main cannot have any parameters.")
{
    CHECK_INVALID(L"fn main(x: i32){}");
}

// TEST_CASE("Global constants.")
// {
//     CHECK_INVALID(L"let var;");
//     CHECK_INVALID(L"let var: i32;");
//     CHECK_VALID(L"let var = 5;");
//     CHECK_VALID(L"let var: i32 = 5;");
// }

// TEST_CASE("Global mutables.")
// {
//     CHECK_INVALID(L"let mut var;");
//     CHECK_VALID(L"let mut var: i32;");
//     CHECK_VALID(L"let mut var = 5;");
//     CHECK_VALID(L"let mut var: i32 = 5;");
// }

// TEST_CASE("Global constants, invalid - redefinitions.")
// {
//     CHECK_INVALID(L"let var = 5; let var = 7;");
// }

// TEST_CASE("Global constants, invalid - name 'main'.")
// {
//     CHECK_INVALID(L"let main = 5;");
// }

// TEST_CASE("Empty main - valid.")
// {
//     CHECK_VALID(L"fn main(){}");
//     CHECK_VALID(L"fn main()=>!{}");
// }

// TEST_CASE("Empty main - invalid return type signature.")
// {
//     CHECK_INVALID(L"fn main()=>i32{}");
// }

// TEST_CASE("Empty main - no return value.")
// {
//     CHECK_INVALID(L"fn main()=>u8{}");
// }

// TEST_CASE("Valid main, with u8 return.")
// {
//     CHECK_VALID(L"fn main()=>u8{return 8;}");
// }

// TEST_CASE("Main with statements, without needed return.")
// {
//     CHECK_INVALID(L"fn main()=>u8{let mut var : i32;}");
//     CHECK_INVALID(L"fn main()=>u8{let mut var : i32;let mut foo : i32;}");
// }