#include "lexer.hpp"
#include "reader.hpp"
#include <string>

bool operator==(const Token &token, const Token &other)
{
    return token.type == other.type &&
           token.value == other.value;
}
bool operator!=(const Token &token, const Token &other)
{
    return !(token == other);
}
wchar_t Lexer::get_new_char()
{
    return this->last_char = this->reader->get();
}
void Lexer::get_nonempty_char()
{
    while (std::isspace(this->last_char))
    {
        this->get_new_char();
    }
}

Token Lexer::get_token()
{
    this->get_nonempty_char();
    return Token(TokenType::END);
}

wchar_t Lexer::peek_char()
{
    return this->reader->peek();
}
