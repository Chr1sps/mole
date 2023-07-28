#include "exceptions.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace std;

bool compare_tokens(const std::wstring &code, const vector<Token> &tokens)
{
    auto locale = Locale("en_US.utf8");
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
    auto locale = Locale("en_US.utf8");
    auto lexer = Lexer::from_wstring(source);
    while (!lexer->eof())
        lexer->get_token();
}

#define T(type) Token(TokenType::type)
#define V(type, value) Token(TokenType::type, value)

#define COMPARE(source, tokens) REQUIRE(compare_tokens(source, tokens))
#define COMPARE_THROWS(source, exception_type)                                \
    REQUIRE_THROWS_AS(consume_tokens(source), exception_type)
#define LIST(...)                                                             \
    {                                                                         \
        __VA_ARGS__                                                           \
    }

TEST_CASE("Empty code.", "[EOF]")
{
    auto locale = Locale("en_US.utf8");
    COMPARE(L"", LIST(T(END)));
}

TEST_CASE("Single char tokens (with no branching alternatives).", "[OPS][EOF]")
{
    COMPARE(L":", LIST(T(COLON), T(END)));
    COMPARE(L",", LIST(T(COMMA), T(END)));
    COMPARE(L";", LIST(T(SEMICOLON), T(END)));

    COMPARE(L"+", LIST(T(PLUS), T(END)));
    COMPARE(L"-", LIST(T(MINUS), T(END)));
    COMPARE(L"*", LIST(T(STAR), T(END)));
    COMPARE(L"/", LIST(T(SLASH), T(END)));
    COMPARE(L"%", LIST(T(PERCENT), T(END)));
    COMPARE(L"~", LIST(T(BIT_NEG), T(END)));

    COMPARE(L"=", LIST(T(ASSIGN), T(END)));
    COMPARE(L"<", LIST(T(LESS), T(END)));
    COMPARE(L">", LIST(T(GREATER), T(END)));
    COMPARE(L"!", LIST(T(NEG), T(END)));
    COMPARE(L"^", LIST(T(BIT_XOR), T(END)));
    COMPARE(L"&", LIST(T(AMPERSAND), T(END)));
    COMPARE(L"|", LIST(T(BIT_OR), T(END)));

    COMPARE(L"{", LIST(T(L_BRACKET), T(END)));
    COMPARE(L"}", LIST(T(R_BRACKET), T(END)));
    COMPARE(L"(", LIST(T(L_PAREN), T(END)));
    COMPARE(L")", LIST(T(R_PAREN), T(END)));
    COMPARE(L"[", LIST(T(L_SQ_BRACKET), T(END)));
    COMPARE(L"]", LIST(T(R_SQ_BRACKET), T(END)));
}

TEST_CASE("Multiple char tokens.", "[LONG][EOF]")
{
    COMPARE(L"++", LIST(T(INCREMENT), T(END)));
    COMPARE(L"+=", LIST(T(ASSIGN_PLUS), T(END)));
    COMPARE(L"--", LIST(T(DECREMENT), T(END)));
    COMPARE(L"-=", LIST(T(ASSIGN_MINUS), T(END)));
    COMPARE(L"*=", LIST(T(ASSIGN_STAR), T(END)));
    COMPARE(L"/=", LIST(T(ASSIGN_SLASH), T(END)));
    COMPARE(L"%=", LIST(T(ASSIGN_PERCENT), T(END)));
    COMPARE(L"~=", LIST(T(ASSIGN_BIT_NEG), T(END)));
    COMPARE(L"=>", LIST(T(LAMBDA_ARROW), T(END)));
    COMPARE(L"==", LIST(T(EQUAL), T(END)));
    COMPARE(L"<=", LIST(T(LESS_EQUAL), T(END)));
    COMPARE(L">=", LIST(T(GREATER_EQUAL), T(END)));
    COMPARE(L"!=", LIST(T(NOT_EQUAL), T(END)));
    COMPARE(L"^=", LIST(T(ASSIGN_BIT_XOR), T(END)));
    COMPARE(L"^^", LIST(T(EXP), T(END)));
    COMPARE(L"^^=", LIST(T(ASSIGN_EXP), T(END)));
    COMPARE(L"&=", LIST(T(ASSIGN_AMPERSAND), T(END)));
    COMPARE(L"&&", LIST(T(AND), T(END)));
    COMPARE(L"|=", LIST(T(ASSIGN_BIT_OR), T(END)));
    COMPARE(L"||", LIST(T(OR), T(END)));
    COMPARE(L"<<", LIST(T(SHIFT_LEFT), T(END)));
    COMPARE(L">>", LIST(T(SHIFT_RIGHT), T(END)));
    COMPARE(L"<<=", LIST(T(ASSIGN_SHIFT_LEFT), T(END)));
    COMPARE(L">>=", LIST(T(ASSIGN_SHIFT_RIGHT), T(END)));
}

TEST_CASE("Comments.", "[COMM][EOF]")
{
    COMPARE(L"//", LIST(T(END)));
    COMPARE(L"// one 2 _three four\n", LIST(T(END)));
    COMPARE(L"/* fn extern main */", LIST(T(END)));
    COMPARE(L"/*\n*/", LIST(T(END)));
    COMPARE(L"/***/", LIST(T(END)));
}

TEST_CASE("Numericals.", "[NUMS][EOF]")
{
    COMPARE(L"1", LIST(V(INT, 1ull), T(END)));
    COMPARE(L"1000", LIST(V(INT, 1000ull), T(END)));
    COMPARE(L"0", LIST(V(INT, 0ull), T(END)));
    COMPARE(L"00001", LIST(V(INT, 1ull), T(END)));
    COMPARE(L"1.0", LIST(V(DOUBLE, 1.0), T(END)));
    COMPARE(L".25", LIST(V(DOUBLE, 0.25), T(END)));
}

TEST_CASE("Expressions.", "[NUMS][OPS][EOF]")
{
    COMPARE(L"1+1", LIST(V(INT, 1ull), T(PLUS), V(INT, 1ull), T(END)));
    COMPARE(L"1.0+1.0", LIST(V(DOUBLE, 1.0), T(PLUS), V(DOUBLE, 1.0), T(END)));
    COMPARE(L"1.+1.", LIST(V(DOUBLE, 1.0), T(PLUS), V(DOUBLE, 1.0), T(END)));
}

TEST_CASE("Keywords.", "[KW][EOF]")
{
    COMPARE(L"fn", LIST(T(KW_FN), T(END)));
    COMPARE(L"extern", LIST(T(KW_EXTERN), T(END)));
    COMPARE(L"mut", LIST(T(KW_MUT), T(END)));
    COMPARE(L"const", LIST(T(KW_CONST), T(END)));
    COMPARE(L"let", LIST(T(KW_LET), T(END)));
    COMPARE(L"return", LIST(T(KW_RETURN), T(END)));
    COMPARE(L"if", LIST(T(KW_IF), T(END)));
    COMPARE(L"else", LIST(T(KW_ELSE), T(END)));
}

TEST_CASE("Type names.", "[TYPE][EOF]")
{
    COMPARE(L"u8", LIST(T(TYPE_U8), T(END)));
    COMPARE(L"u16", LIST(T(TYPE_U16), T(END)));
    COMPARE(L"u32", LIST(T(TYPE_U32), T(END)));
    COMPARE(L"u64", LIST(T(TYPE_U64), T(END)));

    COMPARE(L"i8", LIST(T(TYPE_I8), T(END)));
    COMPARE(L"i16", LIST(T(TYPE_I16), T(END)));
    COMPARE(L"i32", LIST(T(TYPE_I32), T(END)));
    COMPARE(L"i64", LIST(T(TYPE_I64), T(END)));

    COMPARE(L"f32", LIST(T(TYPE_F32), T(END)));
    COMPARE(L"f64", LIST(T(TYPE_F64), T(END)));

    COMPARE(L"char", LIST(T(TYPE_CHAR), T(END)));
}

TEST_CASE("Identifiers.", "[ID][EOF]")
{
    COMPARE(L"name", LIST(V(IDENTIFIER, L"name"), T(END)));
    COMPARE(L"Name", LIST(V(IDENTIFIER, L"Name"), T(END)));
    COMPARE(L"nAmE", LIST(V(IDENTIFIER, L"nAmE"), T(END)));
    COMPARE(L"n4m3", LIST(V(IDENTIFIER, L"n4m3"), T(END)));
    COMPARE(L"snake_case", LIST(V(IDENTIFIER, L"snake_case"), T(END)));
    COMPARE(L"_snake_case", LIST(V(IDENTIFIER, L"_snake_case"), T(END)));
    COMPARE(L"ęóąśłżźćń", LIST(V(IDENTIFIER, L"ęóąśłżźćń"), T(END)));
}

TEST_CASE("Other special tokens.", "[OTHER]")
{
    COMPARE(L"...", LIST(T(ELLIPSIS), T(END)));
    COMPARE(L"_", LIST(T(PLACEHOLDER), T(END)));
}

TEST_CASE("Invalid signs.", "[ERR]")
{
    COMPARE_THROWS(L".", LexerException);
    COMPARE_THROWS(L"..", LexerException);
    COMPARE_THROWS(L"$", LexerException);
    COMPARE_THROWS(L"#", LexerException);
}

TEST_CASE("Assignments.", "[ASGN][KW][ID][OP][EOF]")
{
    COMPARE(L"let name = 0;",
            LIST(T(KW_LET), V(IDENTIFIER, L"name"), T(ASSIGN), V(INT, 0ull),
                 T(SEMICOLON), T(END)));
    COMPARE(L"let mut name: i32 = 0;",
            LIST(T(KW_LET), T(KW_MUT), V(IDENTIFIER, L"name"), T(COLON),
                 T(TYPE_I32), T(ASSIGN), V(INT, 0ull), T(SEMICOLON), T(END)));
    COMPARE(L"let prąd = 1 * 2 + 3 * 4 - 5 >> 6;",
            LIST(T(KW_LET), V(IDENTIFIER, L"prąd"), T(ASSIGN), V(INT, 1ull),
                 T(STAR), V(INT, 2ull), T(PLUS), V(INT, 3ull), T(STAR),
                 V(INT, 4ull), T(MINUS), V(INT, 5ull), T(SHIFT_RIGHT),
                 V(INT, 6ull), T(SEMICOLON), T(END)));
}

TEST_CASE("Function definitions", "[FN][KW][ID][OP][EOF]")
{
    COMPARE(L"fn noop() => {}",
            LIST(T(KW_FN), V(IDENTIFIER, L"noop"), T(L_PAREN), T(R_PAREN),
                 T(LAMBDA_ARROW), T(L_BRACKET), T(R_BRACKET), T(END)));
    COMPARE(L"fn foo(f: f32, i: i32) => i32 {i}",
            LIST(T(KW_FN), V(IDENTIFIER, L"foo"), T(L_PAREN),
                 V(IDENTIFIER, L"f"), T(COLON), T(TYPE_F32), T(COMMA),
                 V(IDENTIFIER, L"i"), T(COLON), T(TYPE_I32), T(R_PAREN),
                 T(LAMBDA_ARROW), T(TYPE_I32), T(L_BRACKET),
                 V(IDENTIFIER, L"i"), T(R_BRACKET), T(END)));
}