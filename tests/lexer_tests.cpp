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
    COMPARE(L"1", LIST(V(I32, 1), T(END)));
}