#include "lexer.hpp"
#include "logger.hpp"
#include "reader.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <string>

#define KEYWORD(wstr, token_type)                                             \
    {                                                                         \
        wstr, TokenType::token_type                                           \
    }

const std::map<std::wstring, TokenType> Lexer::keywords{
    KEYWORD(L"fn", KW_FN),
    KEYWORD(L"extern", KW_EXTERN),
    KEYWORD(L"let", KW_LET),
    KEYWORD(L"return", KW_RETURN),
    KEYWORD(L"mut", KW_MUT),
    KEYWORD(L"const", KW_CONST),
    KEYWORD(L"if", KW_IF),
    KEYWORD(L"else", KW_ELSE),
    KEYWORD(L"while", KW_WHILE),
    KEYWORD(L"match", KW_MATCH),
    KEYWORD(L"continue", KW_CONTINUE),
    KEYWORD(L"break", KW_BREAK),
    KEYWORD(L"as", KW_AS),

    // type names

    KEYWORD(L"u32", TYPE_U32),
    KEYWORD(L"i32", TYPE_I32),
    KEYWORD(L"f64", TYPE_F64),

    KEYWORD(L"char", TYPE_CHAR),
    KEYWORD(L"str", TYPE_STR),

};

#undef KEYWORD

#define CHAR_NODE(wchar, token_type, ...)                                     \
    {                                                                         \
        wchar,                                                                \
        {                                                                     \
            TokenType::token_type,                                            \
            {                                                                 \
                __VA_ARGS__                                                   \
            }                                                                 \
        }                                                                     \
    }
#define EMPTY_NODE(wchar, ...)                                                \
    {                                                                         \
        wchar,                                                                \
        {                                                                     \
            std::nullopt,                                                     \
            {                                                                 \
                __VA_ARGS__                                                   \
            }                                                                 \
        }                                                                     \
    }

const std::map<wchar_t, CharNode> Lexer::char_nodes{
    CHAR_NODE(L':', COLON),
    CHAR_NODE(L',', COMMA),
    CHAR_NODE(L';', SEMICOLON),
    CHAR_NODE(L'+', PLUS, CHAR_NODE(L'+', INCREMENT),
              CHAR_NODE(L'=', ASSIGN_PLUS)),
    CHAR_NODE(L'-', MINUS, CHAR_NODE(L'-', DECREMENT),
              CHAR_NODE(L'=', ASSIGN_MINUS)),
    CHAR_NODE(L'*', STAR, CHAR_NODE(L'=', ASSIGN_STAR)),
    CHAR_NODE(L'%', PERCENT, CHAR_NODE(L'=', ASSIGN_PERCENT)),
    CHAR_NODE(L'~', BIT_NEG, CHAR_NODE(L'=', ASSIGN_BIT_NEG)),
    CHAR_NODE(L'=', ASSIGN, CHAR_NODE(L'>', LAMBDA_ARROW),
              CHAR_NODE(L'=', EQUAL)),
    CHAR_NODE(L'<', LESS, CHAR_NODE(L'=', LESS_EQUAL),
              CHAR_NODE(L'<', SHIFT_LEFT, CHAR_NODE(L'=', ASSIGN_SHIFT_LEFT))),
    CHAR_NODE(
        L'>', GREATER, CHAR_NODE(L'=', GREATER_EQUAL),
        CHAR_NODE(L'>', SHIFT_RIGHT, CHAR_NODE(L'=', ASSIGN_SHIFT_RIGHT))),
    CHAR_NODE(L'!', NEG, CHAR_NODE(L'=', NOT_EQUAL)),
    CHAR_NODE(L'^', BIT_XOR, CHAR_NODE(L'=', ASSIGN_BIT_XOR),
              CHAR_NODE(L'^', EXP, CHAR_NODE(L'=', ASSIGN_EXP))),
    CHAR_NODE(L'&', AMPERSAND, CHAR_NODE(L'=', ASSIGN_AMPERSAND),
              CHAR_NODE(L'&', AND)),
    CHAR_NODE(L'|', BIT_OR, CHAR_NODE(L'=', ASSIGN_BIT_OR),
              CHAR_NODE(L'|', OR)),
    CHAR_NODE(L'{', L_BRACKET),
    CHAR_NODE(L'}', R_BRACKET),
    CHAR_NODE(L'(', L_PAREN),
    CHAR_NODE(L')', R_PAREN),
    CHAR_NODE(L'[', L_SQ_BRACKET),
    CHAR_NODE(L']', R_SQ_BRACKET),
    CHAR_NODE(L'_', PLACEHOLDER),
    EMPTY_NODE(L'.', EMPTY_NODE(L'.', CHAR_NODE(L'.', ELLIPSIS))),
};

#undef CHAR_NODE
#undef EMPTY_NODE

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
        return Token(this->keywords.at(name), this->token_position);
    return Token(TokenType::IDENTIFIER, name, this->token_position);
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

Token Lexer::parse_floating_remainder(std::string &num_str)
{
    num_str += '.';
    this->get_new_char();
    num_str += this->parse_digits();
    double value = std::strtod(num_str.c_str(), 0);
    return Token(TokenType::DOUBLE, value, this->token_position);
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
        auto value = std::stoull(num_str.c_str(), 0);
        return Token(TokenType::INT, value, this->token_position);
    }
}

Token Lexer::parse_operator()
{
    auto node = this->char_nodes.at(this->last_char.value());
    TokenType return_type;
    for (auto next_char = this->peek_char();;
         this->get_new_char(), next_char = this->peek_char())
    {
        if (next_char.has_value() && node.children.contains(next_char.value()))
        {
            node = node.children.at(next_char.value());
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
    return Token(return_type, this->token_position);
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
            return std::optional<Token>(
                Token(TokenType::ASSIGN_SLASH, this->token_position));
        }
    }
    return std::optional<Token>(Token(TokenType::SLASH, this->token_position));
}

std::optional<Token> Lexer::parse_slash()
{
    auto result = this->parse_possible_slash_token();
    if (result)
        return *result;
    else
        return this->get_token();
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
            this->get_new_char();
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

LexerPtr Lexer::from_file(const std::string &path)
{
    ReaderPtr reader = std::make_unique<FileReader>(path, std::locale());
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
        return Token(TokenType::PLACEHOLDER, this->token_position);
    }
}

std::optional<Token> Lexer::get_token()
{
    this->get_nonempty_char();
    this->token_position = this->reader->get_position();
    if (this->last_char == std::nullopt)
        return std::nullopt;
    else if (this->is_a_number_char())
        return this->parse_number_token();
    else if (this->last_char == std::make_optional<wchar_t>(L'_'))
        return this->parse_underscore();
    else if (this->is_identifier_char())
        return this->parse_alpha_token();
    else if (this->last_char.value() == L'/')
        return this->parse_slash();
    else if (this->is_an_operator_char())
        return this->parse_operator();
    else
        return this->report_error(L"invalid char");
}

// the Lexer::is_*() functions below are called with an assumption, that
// the last_char optional has a value

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
    auto error_text = build_wstring(
        L"[ERROR] Lexer error at [", this->reader->get_position().line, ",",
        this->reader->get_position().column, "]: ", msg, ".");
    auto log_msg = LogMessage(error_text, LogLevel::ERROR);
    for (auto logger : this->loggers)
    {
        logger->log(log_msg);
    }

    return Token(TokenType::INVALID, Position());
}

bool Lexer::eof() const
{
    return this->reader->eof();
}

const Position &Lexer::get_position() const
{
    return this->token_position;
}

void Lexer::add_logger(const LoggerPtr &logger)
{
    this->loggers.push_back(logger);
}

void Lexer::remove_logger(const LoggerPtr &logger)
{
    std::erase(this->loggers, logger);
}