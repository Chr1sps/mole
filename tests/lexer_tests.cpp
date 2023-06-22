#include "lexer.hpp"
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
TEST_CASE("Operators.", "[OPS][EOF]")
{
    COMPARE(L"+", LIST(T(PLUS), T(END)));
    COMPARE(L"-", LIST(T(MINUS), T(END)));
    COMPARE(L"*", LIST(T(STAR), T(END)));
    COMPARE(L"/", LIST(T(SLASH), T(END)));
    COMPARE(L"%", LIST(T(PERCENT), T(END)));
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
}