#include "lexer.hpp"
#include "locale.hpp"
#include "parser.hpp"
#include "semantic_checker.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>

bool check_source(const std::wstring &source)
{
    auto locale = Locale("C.utf8");
    auto parser = Parser(Lexer::from_wstring(source));
    auto program = parser.parse();
    auto checker = SemanticChecker();
    return checker.verify(*program);
}

#define CHECK_VALID(source) REQUIRE(check_source(source))
#define CHECK_INVALID(source) REQUIRE_FALSE(check_source(source))
#define FN_WRAP(source) L"fn wrap(){" + std::wstring(source) + L"}"

TEST_CASE("Variable has no value or type assigned.")
{
    CHECK_INVALID(L"let var;");
    CHECK_INVALID(L"let mut var;");
}

TEST_CASE("Variable's declared type and assigned value's type don't match")
{
    SECTION("No type specifiers.")
    {
        CHECK_VALID(L"let var = 1;");
        CHECK_VALID(L"let var = 1.3;");
        CHECK_VALID(L"let var = true;");
        CHECK_VALID(L"let var = 'a';");
        CHECK_VALID(L"let var = \"a\";");
    }

    SECTION("Globals, normal types.")
    {
        CHECK_VALID(L"let var: u32 = 1;");
        CHECK_VALID(L"let var: f64 = 0.1;");
        CHECK_VALID(L"let var: bool = false;");
        CHECK_VALID(L"let var: char = 'a';");
        CHECK_INVALID(L"let var: u32 = 0.1;");
        CHECK_INVALID(L"let var: f64 = 1;");
        CHECK_INVALID(L"let var: bool = 'a';");
    }

    SECTION("References.")
    {
        CHECK_INVALID(L"let var: &u32 = 1;");
        CHECK_VALID(L"let var: f64 = 0.1;");
        CHECK_VALID(L"let var: bool = false;");
        CHECK_VALID(L"let var: char = 'a';");
        CHECK_INVALID(L"let var: u32 = 0.1;");
        CHECK_INVALID(L"let var: f64 = 1;");
    }

    // SECTION("String.")
    // {
    //     CHECK_VALID(L"let var: &str = \"a\";");
    //     CHECK_INVALID(L"let var: &mut str = \"a\";");
    //     CHECK_INVALID(L"let var: str = \"a\";");
    // }
    // SECTION("Inside a function")
    // {
    //     CHECK_VALID(FN_WRAP(L"let var: i32 = 1;"));
    //     CHECK_VALID(FN_WRAP(L"let var: f64 = 0.1;"));
    //     CHECK_INVALID(FN_WRAP(L"let var: i32 = 0.1;"));
    //     CHECK_INVALID(FN_WRAP(L"let var: f64 = 1;"));
    // }
}

// TEST_CASE("Constant has no value assigned.")
// {
//     CHECK_INVALID(L"let var: i32;");
//     CHECK_VALID(L"let var: i32 = 32;");
// }

// TEST_CASE("Constant cannot be reassigned.")
// {
//     CHECK_INVALID(FN_WRAP(L"let value = 5; value = 4;"));
// }

// TEST_CASE("Using an uninitialized variable in an expression.")
// {
//     CHECK_INVALID(
//         FN_WRAP(L"let mut value: i32; let new_value = (4 + value);"));
// }

// TEST_CASE("Referenced value/function is not in scope.")
// {
//     SECTION("Default cases.")
//     {
//         CHECK_INVALID(
//             L"fn foo(){let value = 5;} fn goo(){let new_value = value;}");
//         // CHECK_INVALID(L"fn foo(){fn boo()=>i32{return 5;}}"
//         //               L"fn goo(){let new_value = boo();}");
//     }
//     SECTION("Function arguments.")
//     {
//         CHECK_VALID(L"fn foo(a: i32, b: i32) => i32 {return a + b;}");
//     }
//     // SECTION("Lambdas.")
//     // {
//     //     CHECK_INVALID(L"fn foo(){fn boo()=>i32{return 5;}}"
//     //                   L"fn goo(){let new_value = boo(2, ...);}");
//     //     CHECK_INVALID(L"fn foo(){fn boo()=>i32{return 5;}}"
//     //                   L"fn goo(){let new_value = boo(2, ...);}");
//     // }
// }

// TEST_CASE("Function's argument type is mismatched.")
// {
//     CHECK_INVALID(L"fn foo(x: i32)=>i32{return 32;}"
//                   L"fn boo(){let value = 0.1; let falue = foo(value);}");
//     CHECK_INVALID(L"fn foo(x: i32)=>i32{return 3;}"
//                   L"fn boo(){let boo = foo(0.1);}");
// }

// TEST_CASE("Function is called with a wrong amount of arguments.")
// {
//     CHECK_INVALID(L"fn foo(x: i32){}"
//                   L"fn boo(){let goo = foo(1,2);}");
//     CHECK_INVALID(L"fn foo(x: i32){}"
//                   L"fn boo(){let goo = foo();}");
// }

// TEST_CASE("Referenced value is not a callable.")
// {
//     CHECK_INVALID(FN_WRAP(L"let foo = 5; let boo = foo();"));
// }

// TEST_CASE("Function doesn't return when its type indicates otherwise.")
// {
//     CHECK_INVALID(L"fn foo(x: i32)=>i32{}");
//     CHECK_VALID(L"fn foo(x: i32)=>!{}");
// }

// TEST_CASE("Function returns the wrong type.")
// {
//     CHECK_INVALID(L"fn foo()=>i32{return 0.1;}");
//     CHECK_INVALID(L"fn foo()=>i32{let value = 0.1; return value;}");
// }

// TEST_CASE("Return statement with no value in a function that does return.")
// {
//     CHECK_INVALID(L"fn foo(x: i32)=>i32{return;}");
// }

// TEST_CASE("Function returns a value when it shouldn't.")
// {
//     CHECK_INVALID(L"fn foo(x: i32){return 5;}");
//     CHECK_INVALID(L"fn foo(x: i32)=>!{return 5;}");
// }

// TEST_CASE("Function parameter cannot shadow a variable/function in scope")
// {
//     CHECK_INVALID(L"let var = 5;"
//                   L"fn foo(var: i32){}");
//     CHECK_INVALID(L"fn var(){}"
//                   L"fn foo(var: i32){}");
// }

// TEST_CASE("Const function cannot reference outside variables that are not "
//           "passed as arguments")
// {
//     CHECK_INVALID(L"let var = 5;"
//                   L"fn const foo(){let bar = var;}");
//     CHECK_INVALID(L"let mut var = 5;"
//                   L"fn const foo(){var = 4;}");
//     CHECK_VALID(L"let var = 5;"
//                 L"fn foo(){let bar = var;}");
//     CHECK_VALID(L"let mut var = 5;"
//                 L"fn foo(){var = 4;}");
//     // CHECK_INVALID(L"fn foo() {"
//     //               L"    let var = 5;"
//     //               L"    fn const boo() {let bar = var;}"
//     //               L"}");
//     // CHECK_INVALID(L"fn foo() {"
//     //               L"    let mut var = 5;"
//     //               L"    fn const boo() {var = 4;}"
//     //               L"}");
//     // CHECK_VALID(L"fn foo() {"
//     //             L"    let var = 5;"
//     //             L"    fn boo() {let bar = var;}"
//     //             L"}");
//     // CHECK_VALID(L"fn foo() {"
//     //             L"    let mut var = 5;"
//     //             L"    fn boo() {var = 4;}"
//     //             L"}");
// }

// TEST_CASE("Variable cannot be called 'main'.")
// {
//     CHECK_INVALID(L"let main: i32;");
//     CHECK_INVALID(L"let main: fn()=>!;");
//     CHECK_INVALID(L"let main: fn()=>u8;");
// }

// TEST_CASE("Main should be of type fn()=>u8 or fn()=>!.")
// {
//     CHECK_INVALID(L"fn main()=>i32{return 3;}");
//     CHECK_INVALID(L"fn main()=>f64{return 0.1;}");
// }

// TEST_CASE("Main cannot have any parameters.")
// {
//     CHECK_INVALID(L"fn main(x: i32){}");
// }

// TEST_CASE("Lambda call expression with a wrong number of arguments.")
// {
//     CHECK_INVALID(L"fn foo(a: i32, b: i32, c: i32){} "
//                   L"fn main(){let val = foo(_, 2);}");
//     CHECK_INVALID(L"fn foo(a: i32, b: i32, c: i32, d: i32){} "
//                   L"fn main(){let val = foo(_, _, 3);}");
//     CHECK_INVALID(L"fn foo(a: i32, b: i32, c: i32, d: i32){} "
//                   L"fn main(){let val = foo(_, _, 3, _, 5);}");
//     CHECK_INVALID(L"fn foo(a: i32, b: i32, c: i32, d: i32){} "
//                   L"fn main(){let val = foo(_, _, _, _, 5, ...);}");
//     CHECK_VALID(L"fn foo(a: i32, b: i32, c: i32){} "
//                 L"fn main(){let val = foo(1, ...);}");
//     CHECK_VALID(L"fn foo(a: i32, b: i32, c: i32){} "
//                 L"fn main(){let val = foo(_, 2, ...);}");
//     CHECK_VALID(L"fn foo(a: i32, b: i32, c: i32, d: i32){} "
//                 L"fn main(){let val = foo(_, 2, _, 4, ...);}");
// }

// TEST_CASE("Lambda call expression has mismatched types.")
// {
//     CHECK_INVALID(L"fn foo(a: i32, b: i32){} "
//                   L"fn main(){let val = foo(_, 2.1);}");
//     CHECK_INVALID(L"fn foo(a: i32, b: i32){} "
//                   L"fn main(){let val = foo(1.05, ...);}");
// }

// TEST_CASE("Chaining function calls.")
// {
//     CHECK_VALID(L"fn foo(a: i32, b: i32) => i32 {return a + b;}"
//                 L"fn main() {"
//                 L"  let boo = foo(32, _);"
//                 L"  let goo = boo(32);"
//                 L"}");
//     CHECK_VALID(L"fn foo(a: i32, b: i32) => i32 {return a + b;}"
//                 L"fn main() {"
//                 L"  let boo = foo(32, _)(32);"
//                 L"}");
// }