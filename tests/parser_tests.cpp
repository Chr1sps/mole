#include "locale.hpp"
#include "logger.hpp"
#include "parser.hpp"
#include <catch2/catch_test_macros.hpp>
#include <memory>

bool check_generated_ast(const std::wstring &source, ProgramPtr &&expected)
{
    auto locale = Locale("C.utf8");
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

template <typename T, typename... Types>
auto make_uniques_vector(Types &&...args)
{
    std::vector<std::unique_ptr<T>> result;
    (result.emplace_back(std::forward<Types>(args)), ...);
    return result;
}

template <typename... Types>
std::vector<std::optional<ExprPtr>> make_lambda_vector(Types &&...args)
{
    std::vector<std::optional<ExprPtr>> result;
    (result.emplace_back(std::forward<Types>(args)), ...);
    return result;
}

#define POS(line, col) Position(line, col)

#define TYPE(type, ref_spec) Type(Type(TypeEnum::type, RefSpecifier::ref_spec))
#define NO_TYPE std::nullopt

#define VAREXPR(name, position)                                               \
    std::make_unique<Expression>(VariableExpr(name, position))

#define I32EXPR(value, position)                                              \
    std::make_unique<Expression>(U32Expr(value, position))
#define F64EXPR(value, position)                                              \
    std::make_unique<Expression>(F64Expr(value, position))
#define STREXPR(value, position)                                              \
    std::make_unique<Expression>(StringExpr(value, position))
#define CHAREXPR(value, position)                                             \
    std::make_unique<Expression>(CharExpr(value, position))
#define BOOLEXPR(value, position)                                             \
    std::make_unique<Expression>(BoolExpr(value, position))

#define BINEXPR(lhs, op, rhs, position)                                       \
    std::make_unique<Expression>(BinaryExpr(lhs, rhs, BinOpEnum::op, position))
#define UNEXPR(expr, op, position)                                            \
    std::make_unique<Expression>(UnaryExpr(expr, UnaryOpEnum::op, position))
#define CALLEXPR(callable, args, position)                                    \
    std::make_unique<Expression>(CallExpr(callable, args, position))
#define LAMBDAEXPR(callable, args, position)                                  \
    std::make_unique<Expression>(LambdaCallExpr(callable, args, position))
#define INDEXEXPR(expr, index, position)                                      \
    std::make_unique<Expression>(IndexExpr(expr, index, position))
#define CASTEXPR(expr, type, position)                                        \
    std::make_unique<Expression>(CastExpr(expr, type, position))

#define LITERALS(...) ARGS(__VA_ARGS__)
#define LARM(literals, block, position)                                       \
    std::make_unique<MatchArm>(LiteralArm(literals, block, position))
#define GARM(condition, block, position)                                      \
    std::make_unique<MatchArm>(GuardArm(condition, block, position))
#define EARM(block, position)                                                 \
    std::make_unique<MatchArm>(ElseArm(block, position))
#define ARMS(...) make_uniques_vector<MatchArm>(__VA_ARGS__)

#define MATCH(expr, arms, position)                                           \
    std::make_unique<Statement>(MatchStmt(expr, arms, position))
#define RETURN(expr, position)                                                \
    std::make_unique<Statement>(ReturnStmt(expr, position))
#define CONTINUE(position) std::make_unique<Statement>(ContinueStmt(position))
#define BREAK(position) std::make_unique<Statement>(BreakStmt(position))
#define ASSIGN(lhs, op, rhs, position)                                        \
    std::make_unique<Statement>(AssignStmt(lhs, AssignType::op, rhs, position))
#define IF(condition, then_stmt, else_stmt, position)                         \
    std::make_unique<Statement>(                                              \
        IfStmt(condition, then_stmt, else_stmt, position))
#define WHILE(condition, statement, position)                                 \
    std::make_unique<Statement>(WhileStmt(condition, statement, position))

#define FUNC(name, params, return_type, block, is_const, position)            \
    std::make_unique<FuncDef>(name, params, return_type, block, is_const,     \
                              position)
#define EXTERN(name, params, return_type, position)                           \
    std::make_unique<ExternDef>(name, params, return_type, position)
#define GLOBAL(name, type, initial_value, is_mut, position)                   \
    std::make_unique<VarDeclStmt>(name, type, initial_value, is_mut, position)
#define VAR(name, type, initial_value, is_mut, position)                      \
    std::make_unique<Statement>(                                              \
        VarDeclStmt(name, type, initial_value, is_mut, position))
#define FUNC_BLOCK(statements, position)                                      \
    std::make_unique<Block>(statements, position)
#define BLOCK(statements, position)                                           \
    std::make_unique<Statement>(Block(statements, position))

#define STMTS(...) make_uniques_vector<Statement>(__VA_ARGS__)

#define PARAM(name, type, position)                                           \
    std::make_unique<Parameter>(name, type, position)
#define PARAMS(...) make_uniques_vector<Parameter>(__VA_ARGS__)

#define ARGS(...) make_uniques_vector<Expression>(__VA_ARGS__)
#define LAMBDAARGS(...) make_uniques_vector<Expression>(__VA_ARGS__)
#define NOARG nullptr

#define GLOBALS(...) make_uniques_vector<VarDeclStmt>(__VA_ARGS__)
#define FUNCTIONS(...) make_uniques_vector<FuncDef>(__VA_ARGS__)
#define EXTERNS(...) make_uniques_vector<ExternDef>(__VA_ARGS__)

#define PROGRAM(globals, functions, externs)                                  \
    std::make_unique<Program>(globals, functions, externs)

#define AST_FN_WRAP(statements)                                               \
    PROGRAM(                                                                  \
        GLOBALS(),                                                            \
        FUNCTIONS(FUNC(L"foo", PARAMS(), NO_TYPE,                             \
                       FUNC_BLOCK(statements, POS(1, 9)), false, POS(1, 1))), \
        EXTERNS())

TEST_CASE("Empty code.")
{
    COMPARE(L"", PROGRAM(GLOBALS(), FUNCTIONS(), EXTERNS()));
}

TEST_CASE("Variables.", "[VARS]")
{
    SECTION("No type, no value;")
    {
        COMPARE(L"let var;", PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE, nullptr,
                                                    false, POS(1, 1))),
                                     FUNCTIONS(), EXTERNS()));
    }
    SECTION("Type.")
    {
        COMPARE(L"let var:i32;",
                PROGRAM(GLOBALS(GLOBAL(L"var", TYPE(I32, NON_REF), nullptr,
                                       false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(L"let var:f64;",
                PROGRAM(GLOBALS(GLOBAL(L"var", TYPE(F64, NON_REF), nullptr,
                                       false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
    }
    SECTION("Value - literals.")
    {
        COMPARE(L"let var=5;",
                PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE, I32EXPR(5, POS(1, 9)),
                                       false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let var=5.25;",
            PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE, F64EXPR(5.25, POS(1, 9)),
                                   false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    }
    SECTION("Type and value.")
    {
        COMPARE(
            L"let var:i32 = 5;",
            PROGRAM(GLOBALS(GLOBAL(L"var", TYPE(I32, NON_REF),
                                   I32EXPR(5, POS(1, 15)), false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    }
    SECTION("Mutables.")
    {
        COMPARE(L"let mut var=5;",
                PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE, I32EXPR(5, POS(1, 13)),
                                       true, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let mut var:i32=5;",
            PROGRAM(GLOBALS(GLOBAL(L"var", TYPE(I32, NON_REF),
                                   I32EXPR(5, POS(1, 17)), true, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    }
    SECTION("UTF-8.")
    {
        COMPARE(L"let 变量=32;", PROGRAM(GLOBALS(GLOBAL(L"变量", NO_TYPE,
                                                        I32EXPR(32, POS(1, 8)),
                                                        false, POS(1, 1))),
                                         FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let μεταβλητή=32;",
            PROGRAM(GLOBALS(GLOBAL(L"μεταβλητή", NO_TYPE,
                                   I32EXPR(32, POS(1, 15)), false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
        COMPARE(L"let 変数=32;", PROGRAM(GLOBALS(GLOBAL(L"変数", NO_TYPE,
                                                        I32EXPR(32, POS(1, 8)),
                                                        false, POS(1, 1))),
                                         FUNCTIONS(), EXTERNS()));
        COMPARE(L"let 변수=32;", PROGRAM(GLOBALS(GLOBAL(L"변수", NO_TYPE,
                                                        I32EXPR(32, POS(1, 8)),
                                                        false, POS(1, 1))),
                                         FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let переменная=32;",
            PROGRAM(GLOBALS(GLOBAL(L"переменная", NO_TYPE,
                                   I32EXPR(32, POS(1, 16)), false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
        COMPARE(L"let चर=32;",
                PROGRAM(GLOBALS(GLOBAL(L"चर", NO_TYPE, I32EXPR(32, POS(1, 8)),
                                       false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let អថេរ=32;",
            PROGRAM(GLOBALS(GLOBAL(L"អថេរ", NO_TYPE, I32EXPR(32, POS(1, 10)),
                                   false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    }
    SECTION("Some errors.")
    {
        THROWS_ERRORS(L"let ;");
        THROWS_ERRORS(L"let var:;");
        THROWS_ERRORS(L"let var:=;");
        THROWS_ERRORS(L"let var=;");
    }
}

TEST_CASE("Binary operators.", "[VARS], [BINOP]")
{
    COMPARE(L"let var=5+5;",
            PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                                   BINEXPR(I32EXPR(5, POS(1, 9)), ADD,
                                           I32EXPR(5, POS(1, 11)), POS(1, 9)),
                                   false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=5/5;",
            PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                                   BINEXPR(I32EXPR(5, POS(1, 9)), DIV,
                                           I32EXPR(5, POS(1, 11)), POS(1, 9)),
                                   false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    SECTION("Nested operators.")
    {
        COMPARE(L"let var=5-5-5;",
                PROGRAM(GLOBALS(GLOBAL(
                            L"var", NO_TYPE,

                            BINEXPR(BINEXPR(I32EXPR(5, POS(1, 9)), SUB,
                                            I32EXPR(5, POS(1, 11)), POS(1, 9)),
                                    SUB, I32EXPR(5, POS(1, 13)), POS(1, 9)),

                            false, POS(1, 1))),
                        FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let var=5+5*5;",
            PROGRAM(GLOBALS(GLOBAL(
                        L"var", NO_TYPE,
                        BINEXPR(I32EXPR(5, POS(1, 9)), ADD,
                                BINEXPR(I32EXPR(5, POS(1, 11)), MUL,
                                        I32EXPR(5, POS(1, 13)), POS(1, 11)),
                                POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let var=5+5*5^^5;",
            PROGRAM(GLOBALS(GLOBAL(
                        L"var", NO_TYPE,
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
            PROGRAM(GLOBALS(GLOBAL(
                        L"var", NO_TYPE,
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
                GLOBALS(GLOBAL(
                    L"var", NO_TYPE,
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
        THROWS_ERRORS(L"let var=5+/5;");
    }
}

TEST_CASE("Unary operators.", "[VARS], [UNOP]")
{
    COMPARE(
        L"let var=!5;",
        PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                               UNEXPR(I32EXPR(5, POS(1, 10)), NEG, POS(1, 9)),
                               false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=~5;",
            PROGRAM(GLOBALS(GLOBAL(
                        L"var", NO_TYPE,
                        UNEXPR(I32EXPR(5, POS(1, 10)), BIT_NEG, POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=-5;",
            PROGRAM(GLOBALS(GLOBAL(
                        L"var", NO_TYPE,
                        UNEXPR(I32EXPR(5, POS(1, 10)), MINUS, POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(
        L"let var=&5;",
        PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                               UNEXPR(I32EXPR(5, POS(1, 10)), REF, POS(1, 9)),
                               false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=&mut 5;",
            PROGRAM(GLOBALS(GLOBAL(
                        L"var", NO_TYPE,
                        UNEXPR(I32EXPR(5, POS(1, 14)), MUT_REF, POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=&mut5;",
            PROGRAM(GLOBALS(GLOBAL(
                        L"var", NO_TYPE,
                        UNEXPR(VAREXPR(L"mut5", POS(1, 10)), REF, POS(1, 9)),
                        false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    SECTION("Nested.")
    {
        COMPARE(
            L"let var=-~!5;",
            PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                                   UNEXPR(UNEXPR(UNEXPR(I32EXPR(5, POS(1, 12)),
                                                        NEG, POS(1, 11)),
                                                 BIT_NEG, POS(1, 10)),
                                          MINUS, POS(1, 9)),
                                   false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
        COMPARE(
            L"let var=&mut *&5;",
            PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                                   UNEXPR(UNEXPR(UNEXPR(I32EXPR(5, POS(1, 16)),
                                                        REF, POS(1, 15)),
                                                 DEREF, POS(1, 14)),
                                          MUT_REF, POS(1, 9)),
                                   false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    }
}

TEST_CASE("Nested operators - mixed.")
{
    COMPARE(
        L"let var=-2+-2;",
        PROGRAM(
            GLOBALS(GLOBAL(
                L"var", NO_TYPE,
                BINEXPR(UNEXPR(I32EXPR(2, POS(1, 10)), MINUS, POS(1, 9)), ADD,
                        UNEXPR(I32EXPR(2, POS(1, 13)), MINUS, POS(1, 12)),
                        POS(1, 9)),
                false, POS(1, 1))),
            FUNCTIONS(), EXTERNS()));
}

TEST_CASE("Function definitions.", "[FUNC]")
{
    SECTION("No params, no return type.")
    {
        COMPARE(L"fn foo(){}",
                PROGRAM(GLOBALS(),
                        FUNCTIONS(FUNC(L"foo", PARAMS(), NO_TYPE,
                                       FUNC_BLOCK(STMTS(), POS(1, 9)), false,
                                       POS(1, 1))),
                        EXTERNS()));
    }
    SECTION("Return type.")
    {
        COMPARE(L"fn foo()=>i32{}",
                PROGRAM(GLOBALS(),
                        FUNCTIONS(FUNC(L"foo", PARAMS(), TYPE(I32, NON_REF),
                                       FUNC_BLOCK(STMTS(), POS(1, 14)), false,
                                       POS(1, 1))),
                        EXTERNS()));
    }
    SECTION("Params.")
    {
        COMPARE(
            L"fn foo(x:i32,y:f64){}",
            PROGRAM(GLOBALS(),
                    FUNCTIONS(FUNC(
                        L"foo",
                        PARAMS(PARAM(L"x", TYPE(I32, NON_REF), POS(1, 8)),
                               PARAM(L"y", TYPE(F64, NON_REF), POS(1, 14))),
                        NO_TYPE, FUNC_BLOCK(STMTS(), POS(1, 20)), false,
                        POS(1, 1))),
                    EXTERNS()));
    }
    SECTION("Combined.")
    {
        COMPARE(
            L"fn foo(x:i32,y:f64)=>i32{}",
            PROGRAM(GLOBALS(),
                    FUNCTIONS(FUNC(
                        L"foo",
                        PARAMS(PARAM(L"x", TYPE(I32, NON_REF), POS(1, 8)),
                               PARAM(L"y", TYPE(F64, NON_REF), POS(1, 14))),
                        TYPE(I32, NON_REF), FUNC_BLOCK(STMTS(), POS(1, 25)),
                        false, POS(1, 1))),
                    EXTERNS()));
    }
    SECTION("Const function.")
    {
        COMPARE(
            L"fn const foo(x:i32,y:f64)=>i32{}",
            PROGRAM(GLOBALS(),
                    FUNCTIONS(FUNC(
                        L"foo",
                        PARAMS(PARAM(L"x", TYPE(I32, NON_REF), POS(1, 14)),
                               PARAM(L"y", TYPE(F64, NON_REF), POS(1, 20))),
                        TYPE(I32, NON_REF), FUNC_BLOCK(STMTS(), POS(1, 31)),
                        true, POS(1, 1))),
                    EXTERNS()));
    }
    SECTION("Nested functions.")
    {
        THROWS_ERRORS(L"fn foo(){fn boo(){}}");
    }
    SECTION("Errors.")
    {
        THROWS_ERRORS(L"fn ()=>i32{}");
        THROWS_ERRORS(L"fn foo()=>{}");
        THROWS_ERRORS(L"fn foo()");
        THROWS_ERRORS(L"fn foo(x)=>{}");
        THROWS_ERRORS(L"fn foo(x:)=>{}");
        THROWS_ERRORS(L"fn foo(x:i32,)=>{}");
        THROWS_ERRORS(L"fn foo(,x:i32)=>{}");
    }
}

TEST_CASE("Scopes in functions.", "[SCOPE]")
{
    COMPARE(
        L"fn foo(){{}}",
        PROGRAM(GLOBALS(),
                FUNCTIONS(FUNC(
                    L"foo", PARAMS(), NO_TYPE,
                    FUNC_BLOCK(STMTS(BLOCK(STMTS(), POS(1, 10))), POS(1, 9)),
                    false, POS(1, 1))),
                EXTERNS()));
}

TEST_CASE("Externs.", "[EXT]")
{
    COMPARE(L"extern foo();",
            PROGRAM(GLOBALS(), FUNCTIONS(),
                    EXTERNS(EXTERN(L"foo", PARAMS(), NO_TYPE, POS(1, 1)))));
    COMPARE(L"extern foo(x:i32) => i32;",
            PROGRAM(GLOBALS(), FUNCTIONS(),
                    EXTERNS(EXTERN(
                        L"foo",
                        PARAMS(PARAM(L"x", TYPE(I32, NON_REF), POS(1, 12))),
                        TYPE(I32, NON_REF), POS(1, 1)))));
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
            PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                                   CALLEXPR(L"foo", ARGS(), POS(1, 9)), false,
                                   POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    COMPARE(L"let var=foo(1,2);",
            PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                                   CALLEXPR(L"foo",
                                            ARGS(I32EXPR(1, POS(1, 13)),
                                                 I32EXPR(2, POS(1, 15))),
                                            POS(1, 9)),
                                   false, POS(1, 1))),
                    FUNCTIONS(), EXTERNS()));
    THROWS_ERRORS(L"let var=foo(1,);");
    THROWS_ERRORS(L"let var=foo(,1);");
    THROWS_ERRORS(L"let var=foo(,);");
}

TEST_CASE("Index expressions.")
{
    COMPARE(
        L"let var=foo[1];",
        PROGRAM(GLOBALS(GLOBAL(L"var", NO_TYPE,
                               INDEXEXPR(VAREXPR(L"foo", POS(1, 9)),
                                         I32EXPR(1, POS(1, 13)), POS(1, 9)),
                               false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
    THROWS_ERRORS(L"let var=foo[];");
}

TEST_CASE("Chaining call/index/lambda call expressions.")
{
    COMPARE(
        L"let var=foo(1)[2];",
        PROGRAM(GLOBALS(GLOBAL(
                    L"var", NO_TYPE,
                    INDEXEXPR(CALLEXPR(L"foo", ARGS(I32EXPR(1, POS(1, 13))),
                                       POS(1, 9)),
                              I32EXPR(2, POS(1, 16)), POS(1, 9)),
                    false, POS(1, 1))),
                FUNCTIONS(), EXTERNS()));
}

TEST_CASE("Match statements.")
{
    SECTION("No arms.")
    {
        THROWS_WRAPPED(L"match(){}");
        COMPARE_WRAPPED(L"match(a){}", STMTS(MATCH(VAREXPR(L"a", POS(1, 15)),
                                                   ARMS(), POS(1, 10))));
    }
    SECTION("Literal arms.")
    {
        THROWS_WRAPPED(L"match(a){1=>}");
        COMPARE_WRAPPED(
            L"match(a){1=>{}}",
            STMTS(MATCH(VAREXPR(L"a", POS(1, 15)),
                        ARMS(LARM(LITERALS(I32EXPR(1, POS(1, 19))),
                                  BLOCK(STMTS(), POS(1, 22)), POS(1, 19))),
                        POS(1, 10))));
        COMPARE_WRAPPED(
            L"match(a){1=>{}2=>{}}",
            STMTS(MATCH(VAREXPR(L"a", POS(1, 15)),
                        ARMS(LARM(LITERALS(I32EXPR(1, POS(1, 19))),
                                  BLOCK(STMTS(), POS(1, 22)), POS(1, 19)),
                             LARM(LITERALS(I32EXPR(2, POS(1, 24))),
                                  BLOCK(STMTS(), POS(1, 27)), POS(1, 24))),
                        POS(1, 10))));
        COMPARE_WRAPPED(
            L"match(a){1|2|3=>{}}",
            STMTS(MATCH(VAREXPR(L"a", POS(1, 15)),
                        ARMS(LARM(LITERALS(I32EXPR(1, POS(1, 19)),
                                           I32EXPR(2, POS(1, 21)),
                                           I32EXPR(3, POS(1, 23))),
                                  BLOCK(STMTS(), POS(1, 26)), POS(1, 19))),
                        POS(1, 10))));
        COMPARE_WRAPPED(
            L"match(a){-1|(2+3)|4=>{}}",
            STMTS(MATCH(
                VAREXPR(L"a", POS(1, 15)),
                ARMS(LARM(
                    LITERALS(UNEXPR(I32EXPR(1, POS(1, 20)), MINUS, POS(1, 19)),
                             BINEXPR(I32EXPR(2, POS(1, 23)), ADD,
                                     I32EXPR(3, POS(1, 25)), POS(1, 22)),
                             I32EXPR(4, POS(1, 28))),
                    BLOCK(STMTS(), POS(1, 26)), POS(1, 19))),
                POS(1, 10))));
    }
    SECTION("Guard arms.")
    {
        THROWS_WRAPPED(L"match(a){if()=>{}}");
        THROWS_WRAPPED(L"match(a){if(2)=>}");
        COMPARE_WRAPPED(
            L"match(a){if(1)=>{}}",
            STMTS(MATCH(VAREXPR(L"a", POS(1, 15)),
                        ARMS(GARM(I32EXPR(1, POS(1, 21)),
                                  BLOCK(STMTS(), POS(1, 26)), POS(1, 19))),
                        POS(1, 10))));
    }
    SECTION("Else arms.")
    {
        THROWS_WRAPPED(L"match(a){else=>}");
        COMPARE_WRAPPED(
            L"match(a){else=>{}}",
            STMTS(MATCH(VAREXPR(L"a", POS(1, 15)),
                        ARMS(EARM(BLOCK(STMTS(), POS(1, 22)), POS(1, 19))),
                        POS(1, 10))));
    }
}

TEST_CASE("If statements.")
{
    THROWS_WRAPPED(L"if(){}");
    THROWS_WRAPPED(L"if(1)");
    THROWS_WRAPPED(L"if(1)else");
    THROWS_WRAPPED(L"if(1){}else");
    THROWS_WRAPPED(L"if(1){}else;");
    THROWS_WRAPPED(L"if(1);else;");
    COMPARE_WRAPPED(L"if(1){}", STMTS(IF(I32EXPR(1, POS(1, 12)),
                                         BLOCK(STMTS(), POS(1, 15)), nullptr,
                                         POS(1, 10))));
    COMPARE_WRAPPED(L"if(1)return;", STMTS(IF(I32EXPR(1, POS(1, 12)),
                                              RETURN(nullptr, POS(1, 15)),
                                              nullptr, POS(1, 10))));
    COMPARE_WRAPPED(
        L"if(1){}else{}",
        STMTS(IF(I32EXPR(1, POS(1, 12)), BLOCK(STMTS(), POS(1, 15)),
                 BLOCK(STMTS(), POS(1, 21)), POS(1, 10))));
    COMPARE_WRAPPED(
        L"if(1)return;else break;",
        STMTS(IF(I32EXPR(1, POS(1, 12)), RETURN(nullptr, POS(1, 15)),
                 BREAK(POS(1, 27)), POS(1, 10))));
    COMPARE_WRAPPED(
        L"if(1)if(2){}else{}else{}",
        STMTS(IF(I32EXPR(1, POS(1, 12)),
                 IF(I32EXPR(2, POS(1, 17)), BLOCK(STMTS(), POS(1, 20)),
                    BLOCK(STMTS(), POS(1, 26)), POS(1, 15)),
                 BLOCK(STMTS(), POS(1, 32)), POS(1, 10))));
}

TEST_CASE("While statements.")
{
    THROWS_WRAPPED(L"while(){}");
    THROWS_WRAPPED(L"while(1)");
    THROWS_WRAPPED(L"while(1);");
    COMPARE_WRAPPED(L"while(1){}",
                    STMTS(WHILE(I32EXPR(1, POS(1, 15)),
                                BLOCK(STMTS(), POS(1, 18)), POS(1, 10))));
    COMPARE_WRAPPED(L"while(1)continue;",
                    STMTS(WHILE(I32EXPR(1, POS(1, 15)), CONTINUE(POS(1, 18)),
                                POS(1, 10))));
}