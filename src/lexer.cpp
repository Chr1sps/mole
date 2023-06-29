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
    } while (std::isalnum(this->last_char, this->locale) ||
             this->last_char == L'_');
    if (this->keywords.contains(name))
        return this->keywords[name];
    return Token(TokenType::IDENTIFIER, name);
}

std::string Lexer::parse_digits()
{
    std::string num_str;
    while (std::isdigit(this->last_char, this->locale))
    {
        num_str += this->last_char;
        this->get_new_char();
    }
    return num_str;
}

Token Lexer::parse_floating_suffix(const double &value)
{
    if (this->last_char == 'f')
    {
        this->get_new_char();
        return Token(TokenType::FLOAT, value);
    }
    else
    {
        if (this->last_char == 'd')
        {
            this->get_new_char();
        }
        return Token(TokenType::DOUBLE, value);
    }
}

Token Lexer::parse_floating_remainder(std::string &num_str)
{
    num_str += '.';
    this->get_new_char();
    num_str += this->parse_digits();
    double value = std::strtod(num_str.c_str(), 0);
    return this->parse_floating_suffix(value);
}

Token Lexer::parse_number_token()
{
    std::string num_str;
    num_str += this->parse_digits();
    if (this->last_char == L'.')
    {
        return this->parse_floating_remainder(num_str);
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

std::optional<Token> Lexer::parse_slash()
{
    this->get_new_char();
    if (this->last_char == L'/')
    {
        this->get_new_char();
        this->skip_line_comment();
    }
    else if (this->last_char == L'*')
    {
        this->get_new_char();
        this->skip_block_comment();
    }
    else if (this->last_char == L'=')
    {
        this->get_new_char();
        return std::optional<Token>(Token(TokenType::ASSIGN_SLASH));
    }
    else
        return std::optional<Token>(Token(TokenType::SLASH));
    return std::optional<Token>();
}

void Lexer::skip_line_comment()
{
    while (!this->reader->eof() && this->last_char != L'\n' &&
           this->last_char != L'\r')
    {
        this->get_new_char();
    }
}

void Lexer::skip_block_comment()
{
    for (;;)
    {
        while (!this->reader->eof() && this->last_char != L'*')
        {
            this->get_new_char();
        }
        if (this->peek_char() == L'/')
        {
            this->get_new_char();
            this->get_new_char();
            return;
        }
        this->get_new_char();
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
    if (this->is_a_number_char())
    {
        return this->parse_number_token();
    }
    else if (this->is_identifier_char())
    {
        return this->parse_alpha_token();
    }
    else if (this->last_char == L'/')
    {
        auto result = this->parse_slash();
        if (result.has_value())
            return result.value();
        else
            return this->get_token();
    }
    else if (this->is_an_operator_char())
    {
        return this->parse_operator();
    }
    else if (static_cast<wint_t>(this->last_char) == WEOF)
    {
        return Token(TokenType::END);
    }
    else // throw LexerException(std::string("Invalid char:
         // ").append(this->last_char));
        throw LexerException("Invalid char.");
}

bool Lexer::is_a_number_char()
{
    return std::isdigit(this->last_char, this->locale) ||
           (this->last_char == L'.' &&
            std::isdigit(this->peek_char(), this->locale));
}

bool Lexer::is_identifier_char()
{
    return std::isalpha(this->last_char, this->locale) ||
           this->last_char == L'_';
}

bool Lexer::is_an_operator_char()
{
    return this->char_nodes.contains(this->last_char);
}

wchar_t Lexer::peek_char()
{
    return this->reader->peek();
}

bool Lexer::eof()
{
    return this->reader->eof();
}
