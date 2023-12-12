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
#define FN_WRAP(source) L"fn foo(){" + std::wstring(source) + L"}"
// Remember to add 9 to any position value
#define COMPARE_WRAPPED(source, expected)                                     \
    REQUIRE(check_generated_ast(FN_WRAP(source), AST_FN_WRAP(expected)))
#define THROWS_WRAPPED(source) REQUIRE(throws_errors(FN_WRAP(source)))

// #define REPR_CHECK(source) REQUIRE(compare_output(source, source))
// #define CHECK_EXCEPTION(source, exception)
//     REQUIRE_THROWS_AS(parse_source(source), exception)
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

#define PARENEXPR(value, position) std::make_unique<ParenExpr>(value, position)

#define LITERALS(...) ARGS(__VA_ARGS__)
#define LARM(literals, block, position)                                       \
    std::make_unique<LiteralArm>(literals, block, position)
#define GARM(condition, block, position)                                      \
    std::make_unique<GuardArm>(condition, block, position)
#define PARM(block, position) std::make_unique<PlaceholderArm>(block, position)
#define ARMS(...) make_uniques_vector<MatchArm>(__VA_ARGS__)

#define MATCH(expr, arms, position)                                           \
    std::make_unique<MatchStmt>(expr, arms, position)
#define RETURN(expr, position) std::make_unique<ReturnStmt>(expr, position)
#define CONTINUE(position) std::make_unique<ContinueStmt>(position)
#define BREAK(position) std::make_unique<BreakStmt>(position)
#define ASSIGN(lhs, op, rhs, position)                                        \
    std::make_unique<AssignStmt>(lhs, AssignType::op, rhs, position)
#define VAR(name, type, initial_value, is_mut, position)                      \
    std::make_unique<VarDeclStmt>(name, type, initial_value, is_mut, position)
#define FUNC(name, params, return_type, block, is_const, position)            \
    std::make_unique<FuncDefStmt>(name, params, return_type, block, is_const, \
                                  position)
#define EXTERN(name, params, return_type, position)                           \
    std::make_unique<ExternStmt>(name, params, return_type, position)
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

#define AST_FN_WRAP(statements)                                               \
    PROGRAM(GLOBALS(),                                                        \
            FUNCTIONS(FUNC(L"foo", PARAMS(), nullptr,                         \
                           BLOCK(statements, POS(1, 9)), false, POS(1, 1))),  \
            EXTERNS())

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
    COMPARE(
        L"let var=-5;",
        PROGRAM(GLOBALS(VAR(L"var", nullptr,
                            UNEXPR(I32EXPR(5, POS(1, 10)), MINUS, POS(1, 9)),
                            false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
    SECTION("Nested.")
    {
        COMPARE(
            L"let var=++---~!5;",
            PROGRAM(
                GLOBALS(VAR(
                    L"var", nullptr,
                    UNEXPR(UNEXPR(UNEXPR(UNEXPR(UNEXPR(I32EXPR(5, POS(1, 16)),
                                                       NEG, POS(1, 15)),
                                                BIT_NEG, POS(1, 14)),
                                         MINUS, POS(1, 13)),
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
    COMPARE(
        L"let foo=boo@(_,_,_,_);",
        PROGRAM(GLOBALS(VAR(L"foo", nullptr,
                            LAMBDAEXPR(VAREXPR(L"boo", POS(1, 9)),
                                       LAMBDAARGS(NOARG, NOARG, NOARG, NOARG),
                                       POS(1, 9)),
                            false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
    COMPARE(L"let foo=boo@();",
            PROGRAM(GLOBALS(VAR(L"foo", nullptr,
                                LAMBDAEXPR(VAREXPR(L"boo", POS(1, 9)),
                                           LAMBDAARGS(), POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
}

TEST_CASE("Scopes in functions.", "[SCOPE]")
{
    COMPARE(L"fn foo(){{}}",
            PROGRAM(GLOBALS(),
                    FUNCTIONS(FUNC(
                        L"foo", PARAMS(), nullptr,
                        BLOCK(STMTS(BLOCK(STMTS(), POS(1, 10))), POS(1, 9)),
                        false, POS(1, 1))),
                    EXTERNS()));
}

TEST_CASE("Externs.", "[EXT]")
{
    COMPARE(L"extern foo();",
            PROGRAM(GLOBALS(), FUNCTIONS(),
                    EXTERNS(EXTERN(L"foo", PARAMS(), nullptr, POS(1, 1)))));
    COMPARE(L"extern foo(x:i32) => i32;",
            PROGRAM(GLOBALS(), FUNCTIONS(),
                    EXTERNS(EXTERN(
                        L"foo",
                        PARAMS(PARAM(L"x", STYPE(I32, NON_REF), POS(1, 12))),
                        STYPE(I32, NON_REF), POS(1, 1)))));
}

TEST_CASE("Assign statements.")
{
    COMPARE_WRAPPED(L"x=5;",
                    STMTS(ASSIGN(VAREXPR(L"x", POS(1, 10)), NORMAL,
                                 I32EXPR(5, POS(1, 12)), POS(1, 10))));
    COMPARE_WRAPPED(L"x+=5;",
                    STMTS(ASSIGN(VAREXPR(L"x", POS(1, 10)), PLUS,
                                 I32EXPR(5, POS(1, 13)), POS(1, 10))));
    COMPARE_WRAPPED(L"x-=5;",
                    STMTS(ASSIGN(VAREXPR(L"x", POS(1, 10)), MINUS,
                                 I32EXPR(5, POS(1, 13)), POS(1, 10))));
    COMPARE_WRAPPED(L"x*=5;",
                    STMTS(ASSIGN(VAREXPR(L"x", POS(1, 10)), MUL,
                                 I32EXPR(5, POS(1, 13)), POS(1, 10))));
    COMPARE_WRAPPED(L"x/=5;",
                    STMTS(ASSIGN(VAREXPR(L"x", POS(1, 10)), DIV,
                                 I32EXPR(5, POS(1, 13)), POS(1, 10))));
    COMPARE_WRAPPED(L"x%=5;",
                    STMTS(ASSIGN(VAREXPR(L"x", POS(1, 10)), MOD,
                                 I32EXPR(5, POS(1, 13)), POS(1, 10))));
    COMPARE_WRAPPED(L"x^^=5;",
                    STMTS(ASSIGN(VAREXPR(L"x", POS(1, 10)), EXP,
                                 I32EXPR(5, POS(1, 14)), POS(1, 10))));
}

TEST_CASE("Return statements.")
{
    COMPARE_WRAPPED(L"return;", STMTS(RETURN(nullptr, POS(1, 10))));
    COMPARE_WRAPPED(L"return 8;",
                    STMTS(RETURN(I32EXPR(8, POS(1, 17)), POS(1, 10))));
}

TEST_CASE("Function calls.")
{
    COMPARE(L"let var=foo();",
            PROGRAM(GLOBALS(VAR(L"var", nullptr,
                                CALLEXPR(VAREXPR(L"foo", POS(1, 9)), ARGS(),
                                         POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=foo(1,2);",
            PROGRAM(GLOBALS(VAR(L"var", nullptr,
                                CALLEXPR(VAREXPR(L"foo", POS(1, 9)),
                                         ARGS(I32EXPR(1, POS(1, 13)),
                                              I32EXPR(2, POS(1, 15))),
                                         POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=(foo)();",
            PROGRAM(GLOBALS(VAR(L"var", nullptr,
                                CALLEXPR(PARENEXPR(VAREXPR(L"foo", POS(1, 10)),
                                                   POS(1, 9)),
                                         ARGS(), POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    THROWS_ERRORS(L"let var=foo(1,);");
    THROWS_ERRORS(L"let var=foo(,1);");
    THROWS_ERRORS(L"let var=foo(,);");
}

TEST_CASE("Index expressions.")
{
    COMPARE(L"let var=foo[1];",
            PROGRAM(GLOBALS(VAR(L"var", nullptr,
                                INDEXEXPR(VAREXPR(L"foo", POS(1, 9)),
                                          I32EXPR(1, POS(1, 13)), POS(1, 9)),
                                false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    THROWS_ERRORS(L"let var=foo[];");
}

TEST_CASE("Chaining call/index/lambda call expressions.")
{
    COMPARE(
        L"let var=foo(1)(2);",
        PROGRAM(GLOBALS(VAR(
                    L"var", nullptr,
                    CALLEXPR(CALLEXPR(VAREXPR(L"foo", POS(1, 9)),
                                      ARGS(I32EXPR(1, POS(1, 13))), POS(1, 9)),
                             ARGS(I32EXPR(2, POS(1, 16))), POS(1, 9)),
                    false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=foo@(1)@(2);",
            PROGRAM(GLOBALS(VAR(
                        L"var", nullptr,
                        LAMBDAEXPR(LAMBDAEXPR(VAREXPR(L"foo", POS(1, 9)),
                                              ARGS(I32EXPR(1, POS(1, 14))),
                                              POS(1, 9)),
                                   ARGS(I32EXPR(2, POS(1, 18))), POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=foo[1][2];",
            PROGRAM(GLOBALS(VAR(
                        L"var", nullptr,
                        INDEXEXPR(INDEXEXPR(VAREXPR(L"foo", POS(1, 9)),
                                            I32EXPR(1, POS(1, 13)), POS(1, 9)),
                                  I32EXPR(2, POS(1, 16)), POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(
        L"let var=foo(1)[2]@(3);",
        PROGRAM(GLOBALS(VAR(
                    L"var", nullptr,
                    LAMBDAEXPR(INDEXEXPR(CALLEXPR(VAREXPR(L"foo", POS(1, 9)),
                                                  ARGS(I32EXPR(1, POS(1, 13))),
                                                  POS(1, 9)),
                                         I32EXPR(2, POS(1, 16)), POS(1, 9)),
                               ARGS(I32EXPR(3, POS(1, 20))), POS(1, 9)),
                    false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
}

TEST_CASE("Match statements.")
{
    SECTION("No arms.")
    {
        THROWS_WRAPPED(L"match(){}");
        COMPARE_WRAPPED(
            L"match(a){}",
            STMTS(MATCH(PARENEXPR(VAREXPR(L"a", POS(1, 16)), POS(1, 15)),
                        ARMS(), POS(1, 10))));
    }
    SECTION("Literal arms.")
    {
        THROWS_ERRORS(L"match(a){1=>}");
        COMPARE_WRAPPED(
            L"match(a){1=>{}}",
            STMTS(MATCH(PARENEXPR(VAREXPR(L"a", POS(1, 16)), POS(1, 15)),
                        ARMS(LARM(LITERALS(I32EXPR(1, POS(1, 19))),
                                  BLOCK(STMTS(), POS(1, 22)), POS(1, 19))),
                        POS(1, 10))));
        COMPARE_WRAPPED(
            L"match(a){1=>{}2=>{}}",
            STMTS(MATCH(PARENEXPR(VAREXPR(L"a", POS(1, 16)), POS(1, 15)),
                        ARMS(LARM(LITERALS(I32EXPR(1, POS(1, 19))),
                                  BLOCK(STMTS(), POS(1, 22)), POS(1, 19)),
                             LARM(LITERALS(I32EXPR(2, POS(1, 24))),
                                  BLOCK(STMTS(), POS(1, 27)), POS(1, 24))),
                        POS(1, 10))));
        COMPARE_WRAPPED(
            L"match(a){1|2|3=>{}}",
            STMTS(MATCH(PARENEXPR(VAREXPR(L"a", POS(1, 16)), POS(1, 15)),
                        ARMS(LARM(LITERALS(I32EXPR(1, POS(1, 19)),
                                           I32EXPR(2, POS(1, 21)),
                                           I32EXPR(3, POS(1, 23))),
                                  BLOCK(STMTS(), POS(1, 26)), POS(1, 19))),
                        POS(1, 10))));
        COMPARE_WRAPPED(
            L"match(a){-1|(2+3)|4=>{}}",
            STMTS(MATCH(
                PARENEXPR(VAREXPR(L"a", POS(1, 16)), POS(1, 15)),
                ARMS(LARM(
                    LITERALS(
                        UNEXPR(I32EXPR(1, POS(1, 20)), MINUS, POS(1, 19)),
                        PARENEXPR(BINEXPR(I32EXPR(2, POS(1, 23)), ADD,
                                          I32EXPR(3, POS(1, 25)), POS(1, 23)),
                                  POS(1, 22)),
                        I32EXPR(4, POS(1, 28))),
                    BLOCK(STMTS(), POS(1, 26)), POS(1, 19))),
                POS(1, 10))));
    }
    SECTION("Guard arms.")
    {
        THROWS_ERRORS(L"match(a){if()=>{}}");
        THROWS_ERRORS(L"match(a){if(2)=>}");
        COMPARE_WRAPPED(
            L"match(a){if(1)=>{}}",
            STMTS(
                MATCH(PARENEXPR(VAREXPR(L"a", POS(1, 16)), POS(1, 15)),
                      ARMS(GARM(PARENEXPR(I32EXPR(1, POS(1, 22)), POS(1, 21)),
                                BLOCK(STMTS(), POS(1, 26)), POS(1, 19))),
                      POS(1, 10))));
    }
    SECTION("Placeholder arms.")
    {
        THROWS_ERRORS(L"match(a){_=>}");
        COMPARE_WRAPPED(
            L"match(a){_=>{}}",
            STMTS(MATCH(PARENEXPR(VAREXPR(L"a", POS(1, 16)), POS(1, 15)),
                        ARMS(PARM( BLOCK(STMTS(), POS(1, 22)), POS(1, 19))),
                        POS(1, 10))));
    }
}