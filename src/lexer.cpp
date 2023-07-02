#include "lexer.hpp"
#include "exceptions.hpp"
#include "reader.hpp"
#include "string_builder.hpp"
#include <string>

std::map<std::wstring, Token> Lexer::keywords{
    {L"fn", Token(TokenType::KW_FN)},
    {L"main", Token(TokenType::KW_MAIN)},
    {L"extern", Token(TokenType::KW_EXTERN)},
    {L"mut", Token(TokenType::KW_MUT)},
    {L"const", Token(TokenType::KW_CONST)},
    {L"let", Token(TokenType::KW_LET)},
    {L"return", Token(TokenType::KW_RETURN)},
    {L"if", Token(TokenType::KW_IF)},
    {L"else", Token(TokenType::KW_ELSE)},

    // type names
    {L"f32", Token(TokenType::TYPE_F32)},
    {L"f64", Token(TokenType::TYPE_F64)},

    {L"u8", Token(TokenType::TYPE_U8)},
    {L"u16", Token(TokenType::TYPE_U16)},
    {L"u32", Token(TokenType::TYPE_U32)},
    {L"u64", Token(TokenType::TYPE_U64)},

    {L"i8", Token(TokenType::TYPE_I8)},
    {L"i16", Token(TokenType::TYPE_I16)},
    {L"i32", Token(TokenType::TYPE_I32)},
    {L"i64", Token(TokenType::TYPE_I64)},

    {L"char", Token(TokenType::TYPE_CHAR)},

};

std::map<wchar_t, CharNode> Lexer::char_nodes{
    {L':', {TokenType::COLON, {}}},
    {L',', {TokenType::COMMA, {}}},
    {L';', {TokenType::SEMICOLON, {}}},
    {L'+',
     {TokenType::PLUS,
      {
          {L'+', {TokenType::INCREMENT, {}}},
          {L'=', {TokenType::ASSIGN_PLUS, {}}},
      }}},
    {L'-',
     {TokenType::MINUS,
      {
          {L'-', {TokenType::DECREMENT, {}}},
          {L'=', {TokenType::ASSIGN_MINUS, {}}},
      }}},
    {L'*',
     {TokenType::STAR,
      {
          {L'=', {TokenType::ASSIGN_STAR, {}}},
      }}},
    {L'%',
     {TokenType::PERCENT,
      {
          {L'=', {TokenType::ASSIGN_PERCENT, {}}},
      }}},
    {L'~',
     {TokenType::BIT_NEG,
      {
          {L'=', {TokenType::ASSIGN_BIT_NEG, {}}},
      }}},
    {L'=',
     {TokenType::ASSIGN,
      {
          {L'>', {TokenType::LAMBDA_ARROW, {}}},
          {L'=', {TokenType::EQUAL, {}}},
      }}},
    {L'<',
     {TokenType::LESS,
      {
          {L'=', {TokenType::LESS_EQUAL, {}}},
          {L'<',
           {TokenType::SHIFT_LEFT,
            {
                {L'=', {TokenType::ASSIGN_SHIFT_LEFT, {}}},
            }}},
      }}},
    {L'>',
     {TokenType::GREATER,
      {
          {L'=', {TokenType::GREATER_EQUAL, {}}},
          {L'>',
           {TokenType::SHIFT_RIGHT,
            {
                {L'=', {TokenType::ASSIGN_SHIFT_RIGHT, {}}},
            }}},
      }}},
    {L'!',
     {TokenType::NEG,
      {
          {L'=', {TokenType::NOT_EQUAL, {}}},
      }}},
    {L'^',
     {TokenType::BIT_XOR,
      {
          {L'=', {TokenType::ASSIGN_BIT_XOR, {}}},
          {L'^',
           {TokenType::EXP,
            {
                {L'=', {TokenType::ASSIGN_EXP, {}}},
            }}},
      }}},
    {L'&',
     {TokenType::AMPERSAND,
      {
          {L'=', {TokenType::ASSIGN_AMPERSAND, {}}},
          {L'&', {TokenType::AND, {}}},
      }}},
    {L'|',
     {TokenType::BIT_OR,
      {
          {L'=', {TokenType::ASSIGN_BIT_OR, {}}},
          {L'|', {TokenType::OR, {}}},
      }}},
    {L'{', {TokenType::L_BRACKET, {}}},
    {L'}', {TokenType::R_BRACKET, {}}},
    {L'(', {TokenType::L_PAREN, {}}},
    {L')', {TokenType::R_PAREN, {}}},
    {L'[', {TokenType::L_SQ_BRACKET, {}}},
    {L']', {TokenType::R_SQ_BRACKET, {}}},
    {L'_', {TokenType::PLACEHOLDER, {}}},
    // TODO: fix the abomination below (or not lul)
    {L'.',
     {std::nullopt,
      {{L'.', {std::nullopt, {{L'.', {TokenType::ELLIPSIS, {}}}}}}}}}};

std::optional<wchar_t> Lexer::get_new_char()
{
    return this->last_char = this->reader->get();
}

std::optional<wchar_t> Lexer::get_nonempty_char()
{
    while (this->last_char.has_value() &&
           std::isspace(this->last_char.value(), this->locale))
    {
        this->get_new_char();
    }
    return this->last_char;
}

Token Lexer::parse_alpha_token()
{
    std::wstring name;
    do
    {
        name += this->last_char.value();
        this->get_new_char();
    } while (this->last_char.has_value() &&
             (std::isalnum(this->last_char.value(), this->locale) ||
              this->last_char.value() == L'_'));
    if (this->keywords.contains(name))
        return this->keywords[name];
    return Token(TokenType::IDENTIFIER, name);
}

std::string Lexer::parse_digits()
{
    std::string num_str;
    while (this->last_char.has_value() &&
           std::isdigit(this->last_char.value(), this->locale))
    {
        num_str += this->last_char.value();
        this->get_new_char();
    }
    return num_str;
}

Token Lexer::parse_floating_suffix(const double &value)
{
    if (this->last_char.has_value())
    {
        if (this->last_char.value() == 'f')
        {
            this->get_new_char();
            return Token(TokenType::FLOAT, value);
        }
        else if (this->last_char.value() == 'd')
        {
            this->get_new_char();
        }
    }
    return Token(TokenType::DOUBLE, value);
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
    if (this->last_char.has_value() && this->last_char.value() == L'.')
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
    auto node = this->char_nodes[this->last_char.value()];
    TokenType return_type;
    for (auto next_char = this->peek_char();;
         this->get_new_char(), next_char = this->peek_char())
    {
        if (next_char.has_value() && node.children.contains(next_char.value()))
        {
            node = node.children[next_char.value()];
        }
        else
        {
            try
            {
                return_type = node.type.value();
                this->get_new_char();
                break;
            }
            catch (const std::bad_optional_access &e)
            {
                this->report_error(L"this operator is not supported");
            }
        }
    }
    return Token(return_type);
}

std::optional<Token> Lexer::parse_possible_slash_token()
{
    this->get_new_char();
    if (this->last_char.has_value())
    {
        if (this->last_char.value() == L'/')
        {
            this->get_new_char();
            this->skip_line_comment();
            return std::optional<Token>();
        }
        else if (this->last_char.value() == L'*')
        {
            this->get_new_char();
            this->skip_block_comment();
            return std::optional<Token>();
        }
        else if (this->last_char.value() == L'=')
        {
            this->get_new_char();
            return std::optional<Token>(Token(TokenType::ASSIGN_SLASH));
        }
    }
    return std::optional<Token>(Token(TokenType::SLASH));
}

Token Lexer::parse_slash()
{
    auto result = this->parse_possible_slash_token();
    return result.value_or(this->get_token());
}

void Lexer::skip_line_comment()
{
    while (!this->reader->eof() && this->last_char.value() != L'\n' &&
           this->last_char.value() != L'\r')
    {
        this->get_new_char();
    }
}

void Lexer::skip_block_comment()
{
    for (;;)
    {
        while (!this->reader->eof() && this->last_char.value() != L'*')
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

Token Lexer::parse_underscore()
{
    auto next = this->reader->peek();
    if (next.has_value() && std::isalnum(next.value(), this->locale))
    {
        return this->parse_alpha_token();
    }
    else
    {
        this->get_new_char();
        return Token(TokenType::PLACEHOLDER);
    }
}

Token Lexer::get_token()
{
    this->token_position = this->reader->get_position();
    this->get_nonempty_char();
    if (this->last_char == std::nullopt)
    {
        return Token(TokenType::END);
    }
    else if (this->is_a_number_char())
    {
        return this->parse_number_token();
    }
    else if (this->last_char == std::make_optional<wchar_t>(L'_'))
    {
        return this->parse_underscore();
    }
    else if (this->is_identifier_char())
    {
        return this->parse_alpha_token();
    }
    else if (this->last_char.value() == L'/')
    {
        return this->parse_slash();
    }
    else if (this->is_an_operator_char())
    {
        return this->parse_operator();
    }
    else
        return this->report_error(L"invalid char");
}

// the Lexer::is_*() functions below are called with an assumption, that the
// last_char optional has a value

bool Lexer::is_a_number_char() const
{
    return std::isdigit(this->last_char.value(), this->locale) ||
           (this->last_char.value() == L'.' && this->peek_char().has_value() &&
            std::isdigit(this->peek_char().value(), this->locale));
}

bool Lexer::is_identifier_char() const
{
    return std::isalpha(this->last_char.value(), this->locale) ||
           this->last_char.value() == L'_';
}

bool Lexer::is_an_operator_char() const
{
    return this->char_nodes.contains(this->last_char.value());
}

std::optional<wchar_t> Lexer::peek_char() const
{
    return this->reader->peek();
}

Token Lexer::report_error(const std::wstring &msg)
{
    auto error_msg = build_wstring(
        L"[ERROR] Lexer error at [", this->reader->get_position().line, ",",
        this->reader->get_position().column, "]: ", msg, ".");
    throw LexerException(error_msg);
    return Token(TokenType::INVALID);
}

bool Lexer::eof() const
{
    return this->reader->eof();
}

const Position &Lexer::get_position() const
{
    return this->token_position;
}
