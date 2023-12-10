#include "equation_visitors.hpp"
#include "locale.hpp"
#include "parser.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>

bool check_generated_ast(const std::wstring &source, ProgramPtr &&expected)
{
    auto locale = Locale("en_US.utf8");
    auto parser = Parser(Lexer::from_wstring(source));
    auto result = parser.parse();
    return *result == *expected;
}

#define COMPARE(source, expected)                                             \
    REQUIRE(check_generated_ast(source, expected))

// #define REPR_CHECK(source) REQUIRE(compare_output(source, source))
// #define CHECK_EXCEPTION(source, exception)
//     REQUIRE_THROWS_AS(parse_source(source), exception)
// #define FN_WRAP(source) L"fn foo()=>i32{" + std::wstring(source) + L"}"
// #define COMPARE_STATEMENT(source, output)
//     REQUIRE(compare_output(FN_WRAP(source), FN_WRAP(output)))
// #define REPR_STATEMENT(source) COMPARE_STATEMENT(source, source)

template <typename T, typename... Types>
auto make_uniques_vector(Types &&...args)
{
    std::vector<std::unique_ptr<T>> result;
    (result.emplace_back(std::forward<Types>(args)), ...);
    return result;
}

#define TYPES(...)                                                            \
    std::vector<TypePtr>                                                      \
    {                                                                         \
        __VA_ARGS__                                                           \
    }

#define STYPE(type, ref_spec)                                                 \
    std::make_unique<SimpleType>(TypeEnum::type, RefSpecifier::ref_spec)
#define FTYPE(arg_types, return_type, is_const)                               \
    std::make_unique<FunctionType>(arg_types, return_type, is_const)

#define I32EXPR(value, position) std::make_unique<I32Expr>(value, position)
#define F64EXPR(value, position) std::make_unique<F64Expr>(value, position)

#define POS(line, col) Position(line, col)
#define VAR(name, type, initial_value, is_mut, position)                      \
    std::make_unique<VarDeclStmt>(name, type, initial_value, is_mut, position)
#define GLOBALS(...) make_uniques_vector<VarDeclStmt>(__VA_ARGS__)
#define FUNCTIONS(...)                                                        \
    std::vector<std::unique_ptr<FuncDefStmt>>                                 \
    {                                                                         \
        __VA_ARGS__                                                           \
    }
#define EXTERNS(...)                                                          \
    std::vector<std::unique_ptr<ExternStmt>>                                  \
    {                                                                         \
        __VA_ARGS__                                                           \
    }
#define PROGRAM(globals, functions, externs)                                  \
    std::make_unique<Program>(globals, functions, externs)

TEST_CASE("Empty code.")
{
    COMPARE(L"", PROGRAM(GLOBALS(), FUNCTIONS(), EXTERNS()));
}

TEST_CASE("Variables.", "[VARS]")
{
    SECTION("No type, no value;")
    {
        COMPARE(L"let var;", PROGRAM(GLOBALS(VAR(L"var", nullptr, nullptr,
                                                 false, POS(1, 1))),
                                     FUNCTIONS(), EXTERNS()));
    }
    SECTION("Type.")
    {
        COMPARE(L"let var:i32;",
                PROGRAM(GLOBALS(VAR(L"var", STYPE(I32, NON_REF), nullptr,
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var:f64;",
                PROGRAM(GLOBALS(VAR(L"var", STYPE(F64, NON_REF), nullptr,
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var:fn();",
                PROGRAM(GLOBALS(VAR(L"var", FTYPE(TYPES(), nullptr, false),
                                    nullptr, false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var:fn(i32);",
                PROGRAM(GLOBALS(VAR(
                            L"var",
                            FTYPE(TYPES(STYPE(I32, NON_REF)), nullptr, false),
                            nullptr, false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var:fn(i32)=>i32;",
                PROGRAM(GLOBALS(VAR(L"var",
                                    FTYPE(TYPES(STYPE(I32, NON_REF)),
                                          STYPE(I32, NON_REF), false),
                                    nullptr, false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var:fn(i32,f64)=>i32;",
                PROGRAM(GLOBALS(VAR(L"var",
                                    FTYPE(TYPES(STYPE(I32, NON_REF),
                                                STYPE(F64, NON_REF)),
                                          STYPE(I32, NON_REF), false),
                                    nullptr, false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var:fn const(i32,f64)=>i32;",
                PROGRAM(GLOBALS(VAR(L"var",
                                    FTYPE(TYPES(STYPE(I32, NON_REF),
                                                STYPE(F64, NON_REF)),
                                          STYPE(I32, NON_REF), false),
                                    nullptr, true, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
    }
    SECTION("Value - literals.")
    {
        COMPARE(L"let var=5;",
                PROGRAM(GLOBALS(VAR(L"var", nullptr, I32EXPR(5, POS(1, 9)),
                                    true, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var=5.25;",
                PROGRAM(GLOBALS(VAR(L"var", nullptr, F64EXPR(5.25, POS(1, 9)),
                                    true, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
    }
    SECTION("Type and value.")
    {
        COMPARE(L"let var:i32 = 5;",
                PROGRAM(GLOBALS(VAR(L"var", STYPE(I32, NON_REF),
                                    I32EXPR(5, POS(1, 15)), false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let var:f64 = 5.25;",
            PROGRAM(GLOBALS(VAR(L"var", STYPE(I32, NON_REF),
                                F64EXPR(5.25, POS(1, 15)), false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    }
    SECTION("Mutables.")
    {
        COMPARE(L"let mut var=5;",
                PROGRAM(GLOBALS(VAR(L"var", nullptr, I32EXPR(5, POS(1, 13)),
                                    true, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let mut var:i32=5;",
                PROGRAM(GLOBALS(VAR(L"var", STYPE(I32, NON_REF),
                                    I32EXPR(5, POS(1, 17)), true, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
    }
    SECTION("UTF-8.")
    {
        COMPARE(L"let 变量=32;",
                PROGRAM(GLOBALS(VAR(L"变量", nullptr, I32EXPR(32, POS(1, 8)),
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let μεταβλητή=32;",
            PROGRAM(GLOBALS(VAR(L"μεταβλητή", nullptr, I32EXPR(32, POS(1, 15)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
        COMPARE(L"let 変数=32;",
                PROGRAM(GLOBALS(VAR(L"変数", nullptr, I32EXPR(32, POS(1, 8)),
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let 변수=32;",
                PROGRAM(GLOBALS(VAR(L"변수", nullptr, I32EXPR(32, POS(1, 8)),
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let переменная=32;",
                PROGRAM(GLOBALS(VAR(L"переменная", nullptr,
                                    I32EXPR(32, POS(1, 8)), false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let चर=32;",
                PROGRAM(GLOBALS(VAR(L"चर", nullptr, I32EXPR(32, POS(1, 8)),
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let អថេរ=32;",
                PROGRAM(GLOBALS(VAR(L"អថេរ", nullptr, I32EXPR(32, POS(1, 10)),
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
    }
}

// TEST_CASE("Binary operators.", "[VARS], [BINOP]")
// {
//     COMPARE(L"let var=5+5;", L"let var=(5+5);");
// }

// TEST_CASE("Unary operators.", "[VARS], [UNOP]")
// {
//     COMPARE(L"let var=++5;", L"let var=(++5);");
//     COMPARE(L"let var=--5;", L"let var=(--5);");
//     COMPARE(L"let var=!5;", L"let var=(!5);");
//     COMPARE(L"let var=~5;", L"let var=(~5);");
// }

// TEST_CASE("Nested operators - only binary.")
// {
//     COMPARE(L"let var=2*2*2;", L"let var=((2*2)*2);");
//     COMPARE(L"let var=2+2*2;", L"let var=(2+(2*2));");
//     COMPARE(L"let var=2*2/2%2;", L"let var=(((2*2)/2)%2);");
// }

// TEST_CASE("Nested operators - only unary.")
// {
//     COMPARE(L"let var=--++!~2;", L"let var=(--(++(!(~2))));");
// }

// TEST_CASE("Nested operators - mixed.")
// {
//     COMPARE(L"let var=--2+--2;", L"let var=((--2)+(--2));");
//     COMPARE(L"let var=++1*--2+!3*~4-5>>6;",
//             L"let var=(((((++1)*(--2))+((!3)*(~4)))-5)>>6);");
// }

// TEST_CASE("Function definitions.", "[FUNC]")
// {
//     SECTION("No args, no return type.")
//     {
//         COMPARE(L"fn foo(){}", L"fn foo()=>!{}");
//     }
//     SECTION("Never return type.")
//     {
//         REPR_CHECK(L"fn foo()=>!{}");
//     }
//     SECTION("Return type.")
//     {
//         REPR_CHECK(L"fn foo()=>i32{}");
//         COMPARE(L"fn foo() => fn() {}", L"fn foo()=>fn()=>!{}");
//     }
//     SECTION("Args.")
//     {
//         COMPARE(L"fn foo(x: i32, y: f64) {}", L"fn foo(x:i32,y:f64)=>!{}");
//     }
//     SECTION("Combined.")
//     {
//         REPR_CHECK(L"fn foo(x:i32,y:f64)=>i32{}");
//     }
//     SECTION("Const function.")
//     {
//         REPR_CHECK(L"fn const foo(x:i32,y:f64)=>i32{}");
//     }
//     SECTION("Nested functions.")
//     {
//         CHECK_EXCEPTION(L"fn foo(){fn boo(){}}", ParserException);
//     }
// }

// TEST_CASE("In-place lambdas.")
// {
//     SECTION("Correct.")
//     {
//         REPR_CHECK(L"let foo=boo(1,2,3,4);");
//         REPR_CHECK(L"let foo=boo(1,_,3,_);");
//         REPR_CHECK(L"let foo=boo(1,2,...);");
//         REPR_CHECK(L"let foo=boo(1,_,3,...);");
//         REPR_CHECK(L"let foo=boo(_,2,...);");
//     }
//     SECTION("Exceptions.")
//     {
//         CHECK_EXCEPTION(L"let foo=boo(_,...,_);", ParserException);
//         CHECK_EXCEPTION(L"let foo=boo(1,...,4);", ParserException);
//         CHECK_EXCEPTION(L"let foo=boo(...,3,4);", ParserException);
//         CHECK_EXCEPTION(L"let foo=boo(1,_,...);", ParserException);
//         CHECK_EXCEPTION(L"let foo=boo(...);", ParserException);
//     }
// }

// TEST_CASE("Scopes in functions.", "[SCOPE]")
// {
//     COMPARE(L"fn foo(){{}}", L"fn foo()=>!{{}}");
// }

// TEST_CASE("Externs.", "[EXT]")
// {
//     COMPARE(L"extern foo();", L"extern foo()=>!;");
//     COMPARE(L"extern foo(x:i32);", L"extern foo(x:i32)=>!;");
// }

// TEST_CASE("Assign statements.")
// {
//     REPR_STATEMENT(L"x=5;");
//     REPR_STATEMENT(L"x+=5;");
//     REPR_STATEMENT(L"x-=5;");
//     REPR_STATEMENT(L"x*=5;");
//     REPR_STATEMENT(L"x/=5;");
//     REPR_STATEMENT(L"x%=5;");
//     REPR_STATEMENT(L"x^^=5;");
// }

// TEST_CASE("Return statements.")
// {
//     REPR_STATEMENT(L"return;");
//     REPR_STATEMENT(L"return 8;");
// }

// TEST_CASE("Type tests.")
// {
//     REPR_CHECK(L"let var:i8;");
//     REPR_CHECK(L"let var:i16;");
//     REPR_CHECK(L"let var:i32;");
//     REPR_CHECK(L"let var:i64;");
//     REPR_CHECK(L"let var:u8;");
//     REPR_CHECK(L"let var:u16;");
//     REPR_CHECK(L"let var:u32;");
//     REPR_CHECK(L"let var:u64;");
// }

// TEST_CASE("Function calls.")
// {
//     REPR_STATEMENT(L"let var=foo();");
//     REPR_STATEMENT(L"let var=foo(1,2);");
//     COMPARE_STATEMENT(L"let var=(foo)(1,2);", L"let var=foo(1,2);");
// }

// TEST_CASE("Chaining function calls.")
// {
//     REPR_STATEMENT(L"let var=foo(1,_)(2);");
// }