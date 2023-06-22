#include "lexer.hpp"
#include "reader.hpp"
#include <string>

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
    std::wstring name;
    do
    {
        name += this->last_char;
        this->get_new_char();
    } while (std::isalnum(this->last_char, this->locale) || this->last_char == L'_');
    if (this->keywords.contains(name))
        return this->keywords[name];
    return Token(TokenType::IDENTIFIER, name);
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
        {
            this->get_new_char();
            return Token(TokenType::F64, value);
        }
        else
        {
            if (this->last_char == 'f')
            {
                this->get_new_char();
            }
            return Token(TokenType::F32, value);
        }
    }
    else
    {
        int value = std::stoi(num_str.c_str(), 0);
        return Token(TokenType::INT, value);
    }
}

Token Lexer::parse_operator()
{
    auto node = this->char_nodes[this->last_char];
    TokenType return_type;
    for (auto next_char = this->peek_char();;
         this->get_new_char(), next_char = this->peek_char())
    {
        if (node.children.contains(next_char))
        {
            node = node.children[next_char];
        }
        else
        {
            return_type = node.type;
            this->get_new_char();
            break;
        }
    }
    return Token(return_type);
}

LexerPtr Lexer::from_wstring(const std::wstring &source)
{
    ReaderPtr reader = std::make_unique<StringReader>(source);
    return std::make_unique<Lexer>(reader);
}

Token Lexer::get_token()
{
    this->get_nonempty_char();
    if (std::isdigit(this->last_char) || this->last_char == L'.')
    {
        return this->parse_number_token();
    }
    if (std::isalpha(this->last_char) || this->last_char == L'_')
    {
        return this->parse_alpha_token();
    }
    if (this->char_nodes.contains(this->last_char))
    {
        return this->parse_operator();
    }
    return Token(TokenType::END);
}

wchar_t Lexer::peek_char()
{
    return this->reader->peek();
}

bool Lexer::eof()
{
    return this->reader->eof();
}
