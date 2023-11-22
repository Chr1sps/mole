#include "lexer.hpp"
#include "logger.hpp"
#include "token.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace std;

bool check_log_levels(const std::vector<LogMessage> &messages,
                      const std::vector<LogLevel> &log_levels)
{
    std::vector<LogLevel> output_levels{};
    std::transform(messages.begin(), messages.end(),
                   std::back_inserter(output_levels),
                   [](const LogMessage &msg) { return msg.log_level; });
    return output_levels == log_levels;
}

bool check_lexer(const std::wstring &code, const std::vector<Token> &tokens,
                 const std::vector<LogLevel> &log_levels)
{
    auto logger = std::make_shared<DebugLogger>();
    auto locale = Locale("C.utf8");
    auto lexer = Lexer::from_wstring(code);
    lexer->add_logger(logger.get());

    std::vector<Token> output_tokens;
    for (auto token = lexer->get_token(); token.has_value();
         token = lexer->get_token())
    {
        output_tokens.push_back(*token);
    }
    return (output_tokens == tokens) &&
           (check_log_levels(logger->get_messages(), log_levels));
}

void throws_logs(const std::wstring &code)
{
    auto logger = std::make_shared<DebugLogger>();
    auto locale = Locale("C.utf8");
    auto lexer = Lexer::from_wstring(code);
    lexer->add_logger(logger.get());

    for (; lexer->get_token();)
    {
    }

    REQUIRE_FALSE(logger->get_messages().empty());
}

bool compare_tokens(const std::wstring &code, const vector<Token> &tokens)
{
    return check_lexer(code, tokens, {});
}

#define T(type, line, column) Token(TokenType::type, Position(line, column))
#define V(type, value, line, column)                                          \
    Token(TokenType::type, value, Position(line, column))

#define L(level) LogLevel::level

#define COMPARE(source, tokens) REQUIRE(compare_tokens(source, tokens))
#define COMPARE_FALSE(source, tokens)                                         \
    REQUIRE_FALSE(compare_tokens(source, tokens))
#define COMPARE_WITH_LOG_LEVELS(source, tokens, log_levels)                   \
    REQUIRE(check_lexer(source, tokens, log_levels))
#define LIST(...)                                                             \
    {                                                                         \
        __VA_ARGS__                                                           \
    }

TEST_CASE("Empty code.", "[EOF]")
{
    COMPARE(L"", LIST());
}

TEST_CASE("Single char tokens (with no branching alternatives).", "[OPS]")
{
    COMPARE(L":", LIST(T(COLON, 1, 1)));
    COMPARE(L",", LIST(T(COMMA, 1, 1)));
    COMPARE(L";", LIST(T(SEMICOLON, 1, 1)));

    COMPARE(L"+", LIST(T(PLUS, 1, 1)));
    COMPARE(L"-", LIST(T(MINUS, 1, 1)));
    COMPARE(L"*", LIST(T(STAR, 1, 1)));
    COMPARE(L"/", LIST(T(SLASH, 1, 1)));
    COMPARE(L"%", LIST(T(PERCENT, 1, 1)));
    COMPARE(L"~", LIST(T(BIT_NEG, 1, 1)));

    COMPARE(L"=", LIST(T(ASSIGN, 1, 1)));
    COMPARE(L"<", LIST(T(LESS, 1, 1)));
    COMPARE(L">", LIST(T(GREATER, 1, 1)));
    COMPARE(L"!", LIST(T(NEG, 1, 1)));
    COMPARE(L"^", LIST(T(BIT_XOR, 1, 1)));
    COMPARE(L"&", LIST(T(AMPERSAND, 1, 1)));
    COMPARE(L"|", LIST(T(BIT_OR, 1, 1)));

    COMPARE(L"{", LIST(T(L_BRACKET, 1, 1)));
    COMPARE(L"}", LIST(T(R_BRACKET, 1, 1)));
    COMPARE(L"(", LIST(T(L_PAREN, 1, 1)));
    COMPARE(L")", LIST(T(R_PAREN, 1, 1)));
    COMPARE(L"[", LIST(T(L_SQ_BRACKET, 1, 1)));
    COMPARE(L"]", LIST(T(R_SQ_BRACKET, 1, 1)));
}

TEST_CASE("Multiple char tokens.", "[OPS]")
{
    COMPARE(L"++", LIST(T(INCREMENT, 1, 1)));
    COMPARE(L"+=", LIST(T(ASSIGN_PLUS, 1, 1)));
    COMPARE(L"--", LIST(T(DECREMENT, 1, 1)));
    COMPARE(L"-=", LIST(T(ASSIGN_MINUS, 1, 1)));
    COMPARE(L"*=", LIST(T(ASSIGN_STAR, 1, 1)));
    COMPARE(L"/=", LIST(T(ASSIGN_SLASH, 1, 1)));
    COMPARE(L"%=", LIST(T(ASSIGN_PERCENT, 1, 1)));
    COMPARE(L"~=", LIST(T(ASSIGN_BIT_NEG, 1, 1)));
    COMPARE(L"=>", LIST(T(LAMBDA_ARROW, 1, 1)));
    COMPARE(L"==", LIST(T(EQUAL, 1, 1)));
    COMPARE(L"<=", LIST(T(LESS_EQUAL, 1, 1)));
    COMPARE(L">=", LIST(T(GREATER_EQUAL, 1, 1)));
    COMPARE(L"!=", LIST(T(NOT_EQUAL, 1, 1)));
    COMPARE(L"^=", LIST(T(ASSIGN_BIT_XOR, 1, 1)));
    COMPARE(L"^^", LIST(T(EXP, 1, 1)));
    COMPARE(L"^^=", LIST(T(ASSIGN_EXP, 1, 1)));
    COMPARE(L"&=", LIST(T(ASSIGN_AMPERSAND, 1, 1)));
    COMPARE(L"&&", LIST(T(AND, 1, 1)));
    COMPARE(L"|=", LIST(T(ASSIGN_BIT_OR, 1, 1)));
    COMPARE(L"||", LIST(T(OR, 1, 1)));
    COMPARE(L"<<", LIST(T(SHIFT_LEFT, 1, 1)));
    COMPARE(L">>", LIST(T(SHIFT_RIGHT, 1, 1)));
    COMPARE(L"<<=", LIST(T(ASSIGN_SHIFT_LEFT, 1, 1)));
    COMPARE(L">>=", LIST(T(ASSIGN_SHIFT_RIGHT, 1, 1)));
}

TEST_CASE("Combined operators.")
{
    COMPARE(L"+++", LIST(T(INCREMENT, 1, 1), T(PLUS, 1, 3)));
    COMPARE(L"++++", LIST(T(INCREMENT, 1, 1), T(INCREMENT, 1, 3)));
    COMPARE(L"++==", LIST(T(INCREMENT, 1, 1), T(EQUAL, 1, 3)));
    COMPARE(L">>>=", LIST(T(SHIFT_RIGHT, 1, 1), T(GREATER_EQUAL, 1, 3)));
    COMPARE(L">>=>", LIST(T(ASSIGN_SHIFT_RIGHT, 1, 1), T(GREATER, 1, 4)));
}

TEST_CASE("Comments.", "[COMM]")
{
    COMPARE(L"//", LIST(T(COMMENT, 1, 1)));
    COMPARE(L"// one 2 _three four\n", LIST(T(COMMENT, 1, 1)));
    COMPARE(L"/* fn extern main */", LIST(T(COMMENT, 1, 1)));
    COMPARE(L"/*\n*/", LIST(T(COMMENT, 1, 1)));
    COMPARE(L"/***/", LIST(T(COMMENT, 1, 1)));
}

TEST_CASE("Numericals.", "[NUMS]")
{
    COMPARE(L"1", LIST(V(INT, 1ull, 1, 1)));
    COMPARE(L"1000", LIST(V(INT, 1000ull, 1, 1)));
    COMPARE(L"0", LIST(V(INT, 0ull, 1, 1)));
    COMPARE(L"00001", LIST(V(INT, 1ull, 1, 1)));
    COMPARE(L"1.0", LIST(V(DOUBLE, 1.0, 1, 1)));
    COMPARE(L".25", LIST(V(DOUBLE, 0.25, 1, 1)));
}

TEST_CASE("Expressions.", "[NUMS][OPS]")
{
    COMPARE(L"1+1",
            LIST(V(INT, 1ull, 1, 1), T(PLUS, 1, 2), V(INT, 1ull, 1, 3)));
    COMPARE(L"1.0+1.0",
            LIST(V(DOUBLE, 1.0, 1, 1), T(PLUS, 1, 4), V(DOUBLE, 1.0, 1, 5)));
    COMPARE(L"1.+1.",
            LIST(V(DOUBLE, 1.0, 1, 1), T(PLUS, 1, 3), V(DOUBLE, 1.0, 1, 4)));
}

TEST_CASE("Keywords.", "[KW]")
{
    COMPARE(L"fn", LIST(T(KW_FN, 1, 1)));
    COMPARE(L"extern", LIST(T(KW_EXTERN, 1, 1)));
    COMPARE(L"mut", LIST(T(KW_MUT, 1, 1)));
    COMPARE(L"const", LIST(T(KW_CONST, 1, 1)));
    COMPARE(L"let", LIST(T(KW_LET, 1, 1)));
    COMPARE(L"return", LIST(T(KW_RETURN, 1, 1)));
    COMPARE(L"if", LIST(T(KW_IF, 1, 1)));
    COMPARE(L"else", LIST(T(KW_ELSE, 1, 1)));
    COMPARE(L"while", LIST(T(KW_WHILE, 1, 1)));
    COMPARE(L"match", LIST(T(KW_MATCH, 1, 1)));
    COMPARE(L"continue", LIST(T(KW_CONTINUE, 1, 1)));
    COMPARE(L"break", LIST(T(KW_BREAK, 1, 1)));
    COMPARE(L"as", LIST(T(KW_AS, 1, 1)));
}

TEST_CASE("Type names.", "[TYPE]")
{
    COMPARE(L"u32", LIST(T(TYPE_U32, 1, 1)));
    COMPARE(L"i32", LIST(T(TYPE_I32, 1, 1)));
    COMPARE(L"f64", LIST(T(TYPE_F64, 1, 1)));

    COMPARE(L"char", LIST(T(TYPE_CHAR, 1, 1)));
    COMPARE(L"str", LIST(T(TYPE_STR, 1, 1)));
}

TEST_CASE("Identifiers.", "[ID]")
{
    COMPARE(L"name", LIST(V(IDENTIFIER, L"name", 1, 1)));
    COMPARE(L"Name", LIST(V(IDENTIFIER, L"Name", 1, 1)));
    COMPARE(L"nAmE", LIST(V(IDENTIFIER, L"nAmE", 1, 1)));
    COMPARE(L"n4m3", LIST(V(IDENTIFIER, L"n4m3", 1, 1)));
    COMPARE(L"snake_case", LIST(V(IDENTIFIER, L"snake_case", 1, 1)));
    COMPARE(L"_snake_case", LIST(V(IDENTIFIER, L"_snake_case", 1, 1)));
    COMPARE(L"ęóąśłżźćń", LIST(V(IDENTIFIER, L"ęóąśłżźćń", 1, 1)));
    COMPARE_FALSE(L"3three", LIST(V(IDENTIFIER, L"3three", 1, 1)));
    COMPARE_FALSE(L"3_three", LIST(V(IDENTIFIER, L"3three", 1, 1)));
}

TEST_CASE("Other special tokens.", "[OTHER]")
{
    COMPARE(L"@", LIST(T(AT, 1, 1)));
    COMPARE(L"_", LIST(T(PLACEHOLDER, 1, 1)));
}

TEST_CASE("Invalid signs.", "[ERR]")
{
    COMPARE_WITH_LOG_LEVELS(L".", LIST(T(INVALID, 1, 1)), LIST(L(ERROR)));
    COMPARE_WITH_LOG_LEVELS(L"..", LIST(T(INVALID, 1, 1), T(INVALID, 1, 2)),
                            LIST(L(ERROR), L(ERROR)));
    COMPARE_WITH_LOG_LEVELS(L"$", LIST(T(INVALID, 1, 1)), LIST(L(ERROR)));
    COMPARE_WITH_LOG_LEVELS(L"#", LIST(T(INVALID, 1, 1)), LIST(L(ERROR)));
}

TEST_CASE("Chars.", "[CHAR]")
{
    SECTION("Empty char.")
    {
        COMPARE(L"''", LIST(V(CHAR, L'\0', 1, 1)));
    }
    SECTION("Normal chars.")
    {
        COMPARE(L"'a'", LIST(V(CHAR, L'a', 1, 1)));
        COMPARE(L"'ą'", LIST(V(CHAR, L'ą', 1, 1)));
        COMPARE(L"'😊'", LIST(V(CHAR, L'😊', 1, 1)));
    }
    SECTION("Non-hex escape sequences.")
    {
        COMPARE(L"'\\\\'", LIST(V(CHAR, L'\\', 1, 1)));
        COMPARE(L"'\\n'", LIST(V(CHAR, L'\n', 1, 1)));
        COMPARE(L"'\\t'", LIST(V(CHAR, L'\t', 1, 1)));
        COMPARE(L"'\\r'", LIST(V(CHAR, L'\r', 1, 1)));
        COMPARE(L"'\\\''", LIST(V(CHAR, L'\'', 1, 1)));
        COMPARE(L"'\\\"'", LIST(V(CHAR, L'\"', 1, 1)));
        COMPARE(L"'\\0'", LIST(V(CHAR, L'\0', 1, 1)));
    }
    SECTION("Hex escape sequences.")
    {
        COMPARE(L"'\\{0a}'", LIST(V(CHAR, L'\n', 1, 1)));
        COMPARE(L"'\\{07}'", LIST(V(CHAR, L'\a', 1, 1)));
        COMPARE(L"'\\{0001f60A}'", LIST(V(CHAR, L'😊', 1, 1)));
    }
    SECTION("Invalid chars.")
    {
        throws_logs(L"'");
        throws_logs(L"'''");
        throws_logs(L"'\\'");
        throws_logs(L"'\\{}'");
        throws_logs(L"'\\{aaaaaaaaa}'");
    }
}

TEST_CASE("Strings.", "[STR]")
{
    SECTION("Normal strings.")
    {
        COMPARE(L"\"\"", LIST(V(STRING, L"", 1, 1)));
        COMPARE(L"\"test123\"", LIST(V(STRING, L"test123", 1, 1)));
        COMPARE(L"\"\"\"\"", LIST(V(STRING, L"", 1, 1), V(STRING, L"", 1, 3)));
    }
}

TEST_CASE("Assignments.", "[ASGN][KW][ID][OP]")
{
    COMPARE(L"let name = 0;",
            LIST(T(KW_LET, 1, 1), V(IDENTIFIER, L"name", 1, 5),
                 T(ASSIGN, 1, 10), V(INT, 0ull, 1, 12), T(SEMICOLON, 1, 13)));
    COMPARE(L"let mut name: i32 = 0;",
            LIST(T(KW_LET, 1, 1), T(KW_MUT, 1, 5),
                 V(IDENTIFIER, L"name", 1, 9), T(COLON, 1, 13),
                 T(TYPE_I32, 1, 15), T(ASSIGN, 1, 19), V(INT, 0ull, 1, 21),
                 T(SEMICOLON, 1, 22)));
    COMPARE(L"let prąd = 1 * 2 + 3 * 4 - 5 >> 6;",
            LIST(T(KW_LET, 1, 1), V(IDENTIFIER, L"prąd", 1, 5),
                 T(ASSIGN, 1, 10), V(INT, 1ull, 1, 12), T(STAR, 1, 14),
                 V(INT, 2ull, 1, 16), T(PLUS, 1, 18), V(INT, 3ull, 1, 20),
                 T(STAR, 1, 22), V(INT, 4ull, 1, 24), T(MINUS, 1, 26),
                 V(INT, 5ull, 1, 28), T(SHIFT_RIGHT, 1, 30),
                 V(INT, 6ull, 1, 33), T(SEMICOLON, 1, 34)));
}

TEST_CASE("Function definitions", "[FN][KW][ID][OP]")
{
    COMPARE(L"fn noop() => {}",
            LIST(T(KW_FN, 1, 1), V(IDENTIFIER, L"noop", 1, 4),
                 T(L_PAREN, 1, 8), T(R_PAREN, 1, 9), T(LAMBDA_ARROW, 1, 11),
                 T(L_BRACKET, 1, 14), T(R_BRACKET, 1, 15)));
    COMPARE(L"fn foo(f: f64, i: i32) => i32 {i}",
            LIST(T(KW_FN, 1, 1), V(IDENTIFIER, L"foo", 1, 4), T(L_PAREN, 1, 7),
                 V(IDENTIFIER, L"f", 1, 8), T(COLON, 1, 9), T(TYPE_F64, 1, 11),
                 T(COMMA, 1, 14), V(IDENTIFIER, L"i", 1, 16), T(COLON, 1, 17),
                 T(TYPE_I32, 1, 19), T(R_PAREN, 1, 22), T(LAMBDA_ARROW, 1, 24),
                 T(TYPE_I32, 1, 27), T(L_BRACKET, 1, 31),
                 V(IDENTIFIER, L"i", 1, 32), T(R_BRACKET, 1, 33)));
}