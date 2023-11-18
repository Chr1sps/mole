#include "lexer.hpp"
#include "token.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace std;

bool compare_tokens(const std::wstring &code, const vector<Token> &tokens)
{
    auto locale = Locale("C.utf8");
    auto lexer = Lexer::from_wstring(code);
    for (auto &token : tokens)
    {
        auto actual = lexer->get_token();
        if (actual != token)
            return false;
    }
    return true;
}

void consume_tokens(const std::wstring &source)
{
    auto locale = Locale("C.utf8");
    auto lexer = Lexer::from_wstring(source);
    while (!lexer->eof())
        lexer->get_token();
}

#define T(type, line, column) Token(TokenType::type, Position(line, column))
#define V(type, value, line, column)                                          \
    Token(TokenType::type, value, Position(line, column))

#define COMPARE(source, tokens) REQUIRE(compare_tokens(source, tokens))
#define COMPARE_FALSE(source, tokens)                                         \
    REQUIRE_FALSE(compare_tokens(source, tokens))
#define COMPARE_THROWS(source, exception_type)                                \
    REQUIRE_THROWS_AS(consume_tokens(source), exception_type)
#define LIST(...)                                                             \
    {                                                                         \
        __VA_ARGS__                                                           \
    }

TEST_CASE("Empty code.", "[EOF]")
{
    auto locale = Locale("en_US.utf8");
    COMPARE(L"", LIST());
}

TEST_CASE("Single char tokens (with no branching alternatives).", "[OPS][EOF]")
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

TEST_CASE("Multiple char tokens.", "[LONG][EOF]")
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

TEST_CASE("Comments.", "[COMM][EOF]")
{
    COMPARE(L"//", LIST());
    COMPARE(L"// one 2 _three four\n", LIST());
    COMPARE(L"/* fn extern main */", LIST());
    COMPARE(L"/*\n*/", LIST());
    COMPARE(L"/***/", LIST());
}

TEST_CASE("Numericals.", "[NUMS][EOF]")
{
    COMPARE(L"1", LIST(V(INT, 1ull, 1, 1)));
    COMPARE(L"1000", LIST(V(INT, 1000ull, 1, 1)));
    COMPARE(L"0", LIST(V(INT, 0ull, 1, 1)));
    COMPARE(L"00001", LIST(V(INT, 1ull, 1, 1)));
    COMPARE(L"1.0", LIST(V(DOUBLE, 1.0, 1, 1)));
    COMPARE(L".25", LIST(V(DOUBLE, 0.25, 1, 1)));
}

TEST_CASE("Expressions.", "[NUMS][OPS][EOF]")
{
    COMPARE(L"1+1",
            LIST(V(INT, 1ull, 1, 1), T(PLUS, 1, 2), V(INT, 1ull, 1, 3)));
    COMPARE(L"1.0+1.0",
            LIST(V(DOUBLE, 1.0, 1, 1), T(PLUS, 1, 4), V(DOUBLE, 1.0, 1, 5)));
    COMPARE(L"1.+1.",
            LIST(V(DOUBLE, 1.0, 1, 1), T(PLUS, 1, 3), V(DOUBLE, 1.0, 1, 4)));
}

TEST_CASE("Keywords.", "[KW][EOF]")
{
    COMPARE(L"fn", LIST(T(KW_FN, 1, 1)));
    COMPARE(L"extern", LIST(T(KW_EXTERN, 1, 1)));
    COMPARE(L"mut", LIST(T(KW_MUT, 1, 1)));
    COMPARE(L"const", LIST(T(KW_CONST, 1, 1)));
    COMPARE(L"let", LIST(T(KW_LET, 1, 1)));
    COMPARE(L"return", LIST(T(KW_RETURN, 1, 1)));
    COMPARE(L"if", LIST(T(KW_IF, 1, 1)));
    COMPARE(L"else", LIST(T(KW_ELSE, 1, 1)));
}

TEST_CASE("Type names.", "[TYPE][EOF]")
{
    COMPARE(L"u8", LIST(T(TYPE_U8, 1, 1)));
    COMPARE(L"u16", LIST(T(TYPE_U16, 1, 1)));
    COMPARE(L"u32", LIST(T(TYPE_U32, 1, 1)));
    COMPARE(L"u64", LIST(T(TYPE_U64, 1, 1)));

    COMPARE(L"i8", LIST(T(TYPE_I8, 1, 1)));
    COMPARE(L"i16", LIST(T(TYPE_I16, 1, 1)));
    COMPARE(L"i32", LIST(T(TYPE_I32, 1, 1)));
    COMPARE(L"i64", LIST(T(TYPE_I64, 1, 1)));

    COMPARE(L"f32", LIST(T(TYPE_F32, 1, 1)));
    COMPARE(L"f64", LIST(T(TYPE_F64, 1, 1)));

    COMPARE(L"char", LIST(T(TYPE_CHAR, 1, 1)));
}

TEST_CASE("Identifiers.", "[ID][EOF]")
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
    COMPARE(L"...", LIST(T(ELLIPSIS, 1, 1)));
    COMPARE(L"_", LIST(T(PLACEHOLDER, 1, 1)));
}

TEST_CASE("Invalid signs.", "[ERR]")
{
    // COMPARE_THROWS(L".", LexerException);
    // COMPARE_THROWS(L"..", LexerException);
    // COMPARE_THROWS(L"$", LexerException);
    // COMPARE_THROWS(L"#", LexerException);
}

TEST_CASE("Assignments.", "[ASGN][KW][ID][OP][EOF]")
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

TEST_CASE("Function definitions", "[FN][KW][ID][OP][EOF]")
{
    COMPARE(L"fn noop() => {}",
            LIST(T(KW_FN, 1, 1), V(IDENTIFIER, L"noop", 1, 4),
                 T(L_PAREN, 1, 8), T(R_PAREN, 1, 9), T(LAMBDA_ARROW, 1, 11),
                 T(L_BRACKET, 1, 14), T(R_BRACKET, 1, 15)));
    COMPARE(L"fn foo(f: f32, i: i32) => i32 {i}",
            LIST(T(KW_FN, 1, 1), V(IDENTIFIER, L"foo", 1, 4), T(L_PAREN, 1, 7),
                 V(IDENTIFIER, L"f", 1, 8), T(COLON, 1, 9), T(TYPE_F32, 1, 11),
                 T(COMMA, 1, 14), V(IDENTIFIER, L"i", 1, 16), T(COLON, 1, 17),
                 T(TYPE_I32, 1, 19), T(R_PAREN, 1, 22), T(LAMBDA_ARROW, 1, 24),
                 T(TYPE_I32, 1, 27), T(L_BRACKET, 1, 31),
                 V(IDENTIFIER, L"i", 1, 32), T(R_BRACKET, 1, 33)));
}