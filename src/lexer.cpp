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
    std::string num_str;
    while (std::isdigit(this->last_char, this->locale))
    {
        num_str += this->last_char;
        this->get_new_char();
    }
    if (this->last_char == L'.')
    {
        do
        {
            num_str += this->last_char;
            this->get_new_char();
        } while (std::isdigit(this->last_char, this->locale));

        double value = std::strtod(num_str.c_str(), 0);
        if (this->last_char == 'd')
            return Token(TokenType::F64, value);
        else
            return Token(TokenType::F32, value);
    }
    else
    {
        int value = std::stoi(num_str.c_str(), 0);
        return Token(TokenType::INT, value);
    }
}

LexerPtr Lexer::from_wstring(const std::wstring &source)
{
    ReaderPtr reader = std::make_unique<StringReader>(source);
    return std::make_unique<Lexer>(reader);
}

Token Lexer::get_token()
{
    this->get_nonempty_char();
    if (std::isdigit(this->last_char) || this->last_char == '.')
    {
        return this->parse_number_token();
    }
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
