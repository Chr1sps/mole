#include "lexer.hpp"
#include "token.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace std;

bool compare_tokens(const LexerPtr &lexer, const vector<Token> &tokens)
{
    for (auto &token : tokens)
    {
        auto actual = lexer->get_token();
        if (actual != token)
            return false;
    }
    return true;
}

#define T(type) Token(TokenType::type)
#define V(type, value) Token(TokenType::type, value)

#define COMPARE(source, tokens) REQUIRE(compare_tokens(Lexer::from_wstring(source), tokens))
#define LIST(...)   \
    {               \
        __VA_ARGS__ \
    }

TEST_CASE("Empty code.", "[EOF]")
{
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
    COMPARE(L"&=", LIST(T(ASSIGN_AMPERSAND), T(END)));
    COMPARE(L"&&", LIST(T(AND), T(END)));
    COMPARE(L"|=", LIST(T(ASSIGN_BIT_OR), T(END)));
    COMPARE(L"||", LIST(T(OR), T(END)));
}
TEST_CASE("Comments.", "[COMM][EOF]")
{
    COMPARE(L"//", LIST(T(END)));
    COMPARE(L"// one 2 _three four\n", LIST(T(END)));
    COMPARE(L"/* fn extern main */", LIST(T(END)));
    COMPARE(L"/*\n*/", LIST(T(END)));
}
TEST_CASE("Numericals.", "[NUMS][EOF]")
{
    COMPARE(L"1", LIST(V(INT, 1), T(END)));
    COMPARE(L"1000", LIST(V(INT, 1000), T(END)));
    COMPARE(L"0", LIST(V(INT, 0), T(END)));
    COMPARE(L"00001", LIST(V(INT, 1), T(END)));
    COMPARE(L"1.0", LIST(V(F32, 1.0), T(END)));
    COMPARE(L".25", LIST(V(F32, 0.25), T(END)));
    COMPARE(L"1.0f", LIST(V(F32, 1.0), T(END)));
    COMPARE(L"1.f", LIST(V(F32, 1.0), T(END)));
    COMPARE(L"1.0d", LIST(V(F64, 1.0), T(END)));
    COMPARE(L"1.d", LIST(V(F64, 1.0), T(END)));
}
TEST_CASE("Expressions.", "[NUMS][OPS][EOF]")
{
    COMPARE(L"1+1", LIST(V(INT, 1), T(PLUS), V(INT, 1), T(END)));
    COMPARE(L"1.0+1.0", LIST(V(F32, 1.0), T(PLUS), V(F32, 1.0), T(END)));
    COMPARE(L"1.+1.", LIST(V(F32, 1.0), T(PLUS), V(F32, 1.0), T(END)));
    COMPARE(L"1.f+1.f", LIST(V(F32, 1.0), T(PLUS), V(F32, 1.0), T(END)));
    COMPARE(L"1.d+1.d", LIST(V(F64, 1.0), T(PLUS), V(F64, 1.0), T(END)));
}
TEST_CASE("Keywords.", "[KW][EOF]")
{
    COMPARE(L"fn", LIST(T(KW_FN), T(END)));
    COMPARE(L"main", LIST(T(KW_MAIN), T(END)));
    COMPARE(L"extern", LIST(T(KW_EXTERN), T(END)));
    COMPARE(L"mut", LIST(T(KW_MUT), T(END)));
    COMPARE(L"const", LIST(T(KW_CONST), T(END)));
    COMPARE(L"let", LIST(T(KW_LET), T(END)));
    COMPARE(L"return", LIST(T(KW_RETURN), T(END)));
    COMPARE(L"if", LIST(T(KW_IF), T(END)));
    COMPARE(L"else", LIST(T(KW_ELSE), T(END)));
}
TEST_CASE("Identifiers", "[ID][EOF]")
{
    COMPARE(L"name", LIST(V(IDENTIFIER, L"name"), T(END)));
    COMPARE(L"Name", LIST(V(IDENTIFIER, L"Name"), T(END)));
    COMPARE(L"nAmE", LIST(V(IDENTIFIER, L"nAmE"), T(END)));
    COMPARE(L"n4m3", LIST(V(IDENTIFIER, L"n4m3"), T(END)));
    COMPARE(L"snake_case", LIST(V(IDENTIFIER, L"snake_case"), T(END)));
    COMPARE(L"_snake_case", LIST(V(IDENTIFIER, L"_snake_case"), T(END)));
    COMPARE(L"_", LIST(V(IDENTIFIER, L"_"), T(END)));
}