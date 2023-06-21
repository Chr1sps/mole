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
    while (std::isspace(this->last_char, this->locale))
    {
        this->get_new_char();
    }
}
Token Lexer::parse_alpha_token()
{
    return Token(TokenType::END);
}

Token Lexer::parse_number_token()
{
    return Token();
}

LexerPtr Lexer::from_wstring(const std::wstring &source)
{
    ReaderPtr reader = std::make_unique<StringReader>(source);
    return std::make_unique<Lexer>(reader);
}

Token Lexer::get_token()
{
    this->get_nonempty_char();
    switch (this->last_char)
    {
    case L'+':
        this->get_new_char();
        return Token(TokenType::PLUS);
    case L'-':
        this->get_new_char();
        return Token(TokenType::MINUS);
    case L'*':
        this->get_new_char();
        return Token(TokenType::STAR);
    case L'/':
        this->get_new_char();
        return Token(TokenType::SLASH);
    case L'%':
        this->get_new_char();
        return Token(TokenType::PERCENT);
    default:
        break;
    }
    return Token(TokenType::END);
}

wchar_t Lexer::peek_char()
{
    return this->reader->peek();
}
