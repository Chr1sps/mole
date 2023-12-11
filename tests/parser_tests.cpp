#include "equation_visitors.hpp"
#include "locale.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>

bool check_generated_ast(const std::wstring &source, ProgramPtr &&expected)
{
    auto locale = Locale("en_US.utf8");
    auto parser = Parser(Lexer::from_wstring(source));
    auto logger = DebugLogger();
    parser.add_logger(&logger);
    auto result = parser.parse();
    auto are_same = *result == *expected;
    auto no_errors = logger.get_messages().empty();
    return are_same && no_errors;
}

bool throws_errors(const std::wstring &source)
{
    auto locale = Locale("en_US.utf8");
    auto parser = Parser(Lexer::from_wstring(source));
    auto logger = DebugLogger();
    parser.add_logger(&logger);
    auto result = parser.parse();
    return !logger.get_messages().empty();
}

#define COMPARE(source, expected)                                             \
    REQUIRE(check_generated_ast(source, expected))
#define THROWS_ERRORS(source) REQUIRE(throws_errors(source))

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

template <typename... Types>
std::vector<std::optional<ExprNodePtr>> make_lambda_vector(Types &&...args)
{
    std::vector<std::optional<ExprNodePtr>> result;
    (result.emplace_back(std::forward<Types>(args)), ...);
    return result;
}

#define POS(line, col) Position(line, col)

#define STYPE(type, ref_spec)                                                 \
    std::make_unique<SimpleType>(TypeEnum::type, RefSpecifier::ref_spec)
#define FTYPE(arg_types, return_type, is_const)                               \
    std::make_unique<FunctionType>(arg_types, return_type, is_const)

#define TYPES(...)                                                            \
    std::vector<TypePtr>                                                      \
    {                                                                         \
        __VA_ARGS__                                                           \
    }

#define VAREXPR(name, position) std::make_unique<VariableExpr>(name, position)

#define I32EXPR(value, position) std::make_unique<I32Expr>(value, position)
#define F64EXPR(value, position) std::make_unique<F64Expr>(value, position)
#define STREXPR(value, position) std::make_unique<StringExpr>(value, position)
#define CHAREXPR(value, position) std::make_unique<CharExpr>(value, position)
#define BOOLEXPR(value, position) std::make_unique<BoolExpr>(value, position)

#define BINEXPR(lhs, op, rhs, position)                                       \
    std::make_unique<BinaryExpr>(lhs, rhs, BinOpEnum::op, position)
#define UNEXPR(expr, op, position)                                            \
    std::make_unique<UnaryExpr>(expr, UnaryOpEnum::op, position)
#define CALLEXPR(callable, args, position)                                    \
    std::make_unique<CallExpr>(callable, args, position)
#define LAMBDAEXPR(callable, args, position)                                  \
    std::make_unique<LambdaCallExpr>(callable, args, position)
#define INDEXEXPR(expr, index, position)                                      \
    std::make_unique<IndexExpr>(expr, index, position)
#define CASTEXPR(expr, type, position)                                        \
    std::make_unique<CastExpr>(expr, type, position)

#define VAR(name, type, initial_value, is_mut, position)                      \
    std::make_unique<VarDeclStmt>(name, type, initial_value, is_mut, position)
#define FUNC(name, params, return_type, block, is_const, position)            \
    std::make_unique<FuncDefStmt>(name, params, return_type, block, is_const, \
                                  position)
#define BLOCK(statements, position)                                           \
    std::make_unique<Block>(statements, position)

#define STMTS(...) make_uniques_vector<Statement>(__VA_ARGS__)

#define PARAM(name, type, position)                                           \
    std::make_unique<Parameter>(name, type, position)
#define PARAMS(...) make_uniques_vector<Parameter>(__VA_ARGS__)

#define ARGS(...) make_uniques_vector<ExprNode>(__VA_ARGS__)
#define LAMBDAARGS(...) make_uniques_vector<ExprNode>(__VA_ARGS__)
#define NOARG nullptr

#define GLOBALS(...) make_uniques_vector<VarDeclStmt>(__VA_ARGS__)
#define FUNCTIONS(...) make_uniques_vector<FuncDefStmt>(__VA_ARGS__)
#define EXTERNS(...) make_uniques_vector<ExternStmt>(__VA_ARGS__)

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
                                          STYPE(I32, NON_REF), true),
                                    nullptr, false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
    }
    SECTION("Value - literals.")
    {
        COMPARE(L"let var=5;",
                PROGRAM(GLOBALS(VAR(L"var", nullptr, I32EXPR(5, POS(1, 9)),
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var=5.25;",
                PROGRAM(GLOBALS(VAR(L"var", nullptr, F64EXPR(5.25, POS(1, 9)),
                                    false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
    }
    SECTION("Type and value.")
    {
        COMPARE(L"let var:i32 = 5;",
                PROGRAM(GLOBALS(VAR(L"var", STYPE(I32, NON_REF),
                                    I32EXPR(5, POS(1, 15)), false, POS(1, 1))),
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
        COMPARE(
            L"let переменная=32;",
            PROGRAM(GLOBALS(VAR(L"переменная", nullptr,
                                I32EXPR(32, POS(1, 16)), false, POS(1, 1))),
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

TEST_CASE("Binary operators.", "[VARS], [BINOP]")
{
    COMPARE(L"let var=5+5;",
            PROGRAM(GLOBALS(VAR(L"var", nullptr,
                                BINEXPR(I32EXPR(5, POS(1, 9)), ADD,
                                        I32EXPR(5, POS(1, 11)), POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    SECTION("Nested operators.")
    {
        COMPARE(
            L"let var=5+5*5;",
            PROGRAM(GLOBALS(VAR(
                        L"var", nullptr,
                        BINEXPR(I32EXPR(5, POS(1, 9)), ADD,
                                BINEXPR(I32EXPR(5, POS(1, 11)), MUL,
                                        I32EXPR(5, POS(1, 13)), POS(1, 11)),
                                POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let var=5+5*5^^5;",
            PROGRAM(GLOBALS(VAR(
                        L"var", nullptr,
                        BINEXPR(I32EXPR(5, POS(1, 9)), ADD,
                                BINEXPR(I32EXPR(5, POS(1, 11)), MUL,
                                        BINEXPR(I32EXPR(5, POS(1, 13)), EXP,
                                                I32EXPR(5, POS(1, 16)),
                                                POS(1, 13)),
                                        POS(1, 11)),
                                POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let var=5^^5^^5^^5;",
            PROGRAM(GLOBALS(VAR(
                        L"var", nullptr,
                        BINEXPR(I32EXPR(5, POS(1, 9)), EXP,
                                BINEXPR(I32EXPR(5, POS(1, 12)), EXP,
                                        BINEXPR(I32EXPR(5, POS(1, 15)), EXP,
                                                I32EXPR(5, POS(1, 18)),
                                                POS(1, 15)),
                                        POS(1, 12)),
                                POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let var=5*5*5*5;",
            PROGRAM(
                GLOBALS(VAR(
                    L"var", nullptr,
                    BINEXPR(BINEXPR(BINEXPR(I32EXPR(5, POS(1, 9)), MUL,
                                            I32EXPR(5, POS(1, 11)), POS(1, 9)),
                                    MUL, I32EXPR(5, POS(1, 13)), POS(1, 9)),
                            MUL, I32EXPR(5, POS(1, 15)), POS(1, 9)),
                    false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
    }
    SECTION("Shunting yard errors.")
    {
        THROWS_ERRORS(L"let var=5+;");
        THROWS_ERRORS(L"let var=5+*5;");
    }
}

TEST_CASE("Unary operators.", "[VARS], [UNOP]")
{
    COMPARE(L"let var=++5;",
            PROGRAM(GLOBALS(VAR(L"var", nullptr,
                                UNEXPR(I32EXPR(5, POS(1, 11)), INC, POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=--5;",
            PROGRAM(GLOBALS(VAR(L"var", nullptr,
                                UNEXPR(I32EXPR(5, POS(1, 11)), DEC, POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=!5;",
            PROGRAM(GLOBALS(VAR(L"var", nullptr,
                                UNEXPR(I32EXPR(5, POS(1, 10)), NEG, POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(
        L"let var=~5;",
        PROGRAM(GLOBALS(VAR(L"var", nullptr,
                            UNEXPR(I32EXPR(5, POS(1, 10)), BIT_NEG, POS(1, 9)),
                            false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
    SECTION("Nested.")
    {
        COMPARE(L"let var=++--~!5;",
                PROGRAM(GLOBALS(VAR(
                            L"var", nullptr,
                            UNEXPR(UNEXPR(UNEXPR(UNEXPR(I32EXPR(5, POS(1, 15)),
                                                        NEG, POS(1, 14)),
                                                 BIT_NEG, POS(1, 13)),
                                          DEC, POS(1, 11)),
                                   INC, POS(1, 9)),
                            false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
    }
}

TEST_CASE("Nested operators - mixed.")
{
    COMPARE(
        L"let var=--2+--2;",
        PROGRAM(
            GLOBALS(VAR(
                L"var", nullptr,
                BINEXPR(UNEXPR(I32EXPR(2, POS(1, 11)), DEC, POS(1, 9)), ADD,
                        UNEXPR(I32EXPR(2, POS(1, 15)), DEC, POS(1, 13)),
                        POS(1, 9)),
                false, POS(1, 1))),
            FUNCTIONS(), EXTERNS()));
    // COMPARE(
    //     L"let var=++1;",
    //     PROGRAM(
    //         GLOBALS(VAR(
    //             L"var", nullptr,
    //             BINEXPR(UNEXPR(I32EXPR(2, POS(1, 11)), DEC, POS(1, 9)), ADD,
    //                     UNEXPR(I32EXPR(2, POS(1, 15)), DEC, POS(1, 13)),
    //                     POS(1, 9)),
    //             false, POS(1, 1))),
    //         FUNCTIONS(), EXTERNS()));
    // COMPARE(L"let var=++1*--2+!3*~4-5>>6;",
    //         L"let var=(((((++1)*(--2))+((!3)*(~4)))-5)>>6);");
}

TEST_CASE("Function definitions.", "[FUNC]")
{
    SECTION("No params, no return type.")
    {
        COMPARE(L"fn foo(){}",
                PROGRAM(GLOBALS(),
                        FUNCTIONS(FUNC(L"foo", PARAMS(), nullptr,
                                       BLOCK(STMTS(), POS(1, 9)), false,
                                       POS(1, 1))),
                        EXTERNS()));
    }
    SECTION("Return type.")
    {
        COMPARE(L"fn foo()=>i32{}",
                PROGRAM(GLOBALS(),
                        FUNCTIONS(FUNC(L"foo", PARAMS(), STYPE(I32, NON_REF),
                                       BLOCK(STMTS(), POS(1, 14)), false,
                                       POS(1, 1))),
                        EXTERNS()));
        COMPARE(L"fn foo()=>fn(){}",
                PROGRAM(GLOBALS(),
                        FUNCTIONS(FUNC(
                            L"foo", PARAMS(), FTYPE(TYPES(), nullptr, false),
                            BLOCK(STMTS(), POS(1, 15)), false, POS(1, 1))),
                        EXTERNS()));
    }
    SECTION("Params.")
    {
        COMPARE(
            L"fn foo(x:i32,y:f64){}",
            PROGRAM(
                GLOBALS(),
                FUNCTIONS(FUNC(
                    L"foo",
                    PARAMS(PARAM(L"x", STYPE(I32, NON_REF), POS(1, 8)),
                           PARAM(L"y", STYPE(F64, NON_REF), POS(1, 14))),
                    nullptr, BLOCK(STMTS(), POS(1, 20)), false, POS(1, 1))),
                EXTERNS()));
    }
    SECTION("Combined.")
    {
        COMPARE(
            L"fn foo(x:i32,y:f64)=>i32{}",
            PROGRAM(GLOBALS(),
                    FUNCTIONS(FUNC(
                        L"foo",
                        PARAMS(PARAM(L"x", STYPE(I32, NON_REF), POS(1, 8)),
                               PARAM(L"y", STYPE(F64, NON_REF), POS(1, 14))),
                        STYPE(I32, NON_REF), BLOCK(STMTS(), POS(1, 25)), false,
                        POS(1, 1))),
                    EXTERNS()));
    }
    SECTION("Const function.")
    {
        COMPARE(
            L"fn const foo(x:i32,y:f64)=>i32{}",
            PROGRAM(GLOBALS(),
                    FUNCTIONS(FUNC(
                        L"foo",
                        PARAMS(PARAM(L"x", STYPE(I32, NON_REF), POS(1, 14)),
                               PARAM(L"y", STYPE(F64, NON_REF), POS(1, 20))),
                        STYPE(I32, NON_REF), BLOCK(STMTS(), POS(1, 31)), true,
                        POS(1, 1))),
                    EXTERNS()));
    }
    SECTION("Nested functions.")
    {
        THROWS_ERRORS(L"fn foo(){fn boo(){}}");
    }
}

TEST_CASE("In-place lambdas.")
{
    COMPARE(L"let foo=boo@(1,2,3,4);",
            PROGRAM(GLOBALS(VAR(L"foo", nullptr,
                                LAMBDAEXPR(VAREXPR(L"boo", POS(1, 9)),
                                           LAMBDAARGS(I32EXPR(1, POS(1, 14)),
                                                      I32EXPR(2, POS(1, 16)),
                                                      I32EXPR(3, POS(1, 18)),
                                                      I32EXPR(4, POS(1, 20))),
                                           POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let foo=boo@(1,_,3,_);",
            PROGRAM(GLOBALS(VAR(
                        L"foo", nullptr,
                        LAMBDAEXPR(VAREXPR(L"boo", POS(1, 9)),
                                   LAMBDAARGS(I32EXPR(1, POS(1, 14)), NOARG,
                                              I32EXPR(3, POS(1, 18)), NOARG),
                                   POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
}

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