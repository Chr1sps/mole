#include "lexer.hpp"
#include "logger.hpp"
#include "reader.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <limits>
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
    KEYWORD(L"true", KW_TRUE),
    KEYWORD(L"false", KW_FALSE),

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
    CHAR_NODE(L'@', AT),
};

#undef CHAR_NODE
#undef EMPTY_NODE

Token Lexer::report_and_consume(const std::wstring &msg)
{
    this->report(LogLevel::ERROR, L"Lexer error at [", this->position.line,
                 ",", this->position.column, "]: ", msg, ".");
    auto invalid = Token(TokenType::INVALID, this->position);
    this->get_new_char();
    return invalid;
}

std::optional<wchar_t> Lexer::get_new_char()
{
    std::tie(this->last_char, this->position) = this->reader->get();
    return this->last_char;
}

std::optional<wchar_t> Lexer::get_nonempty_char()
{
    while (this->last_char.has_value() && std::iswspace(*(this->last_char)))
    {
        this->get_new_char();
    }
    return this->last_char;
}

std::optional<Token> Lexer::parse_alpha_or_placeholder(
    const Position &position)
{

    std::wstring name;
    if (this->last_char == L'_')
    {
        name += L'_';
        this->get_new_char();
        if (!this->is_alpha_char())
        {
            return Token(TokenType::PLACEHOLDER, position);
        }
    }
    do
    {
        name += *(this->last_char);
        this->get_new_char();
    } while (this->is_alpha_char() && name.length() < this->max_var_name_size);

    if (auto name_iter = this->keywords.find(name);
        name_iter != this->keywords.end())
        return Token(name_iter->second, position);

    if (name.length() == this->max_var_name_size && this->is_alpha_char())
        return this->report_and_consume(L"variable name length is too long");

    return Token(TokenType::IDENTIFIER, name, position);
}

unsigned int convert_to_int(const wchar_t &chr)
{
    return chr - L'0';
}

std::optional<unsigned long long> Lexer::parse_integral()
{
    unsigned long long result = 0;
    for (; this->last_char.has_value() && std::iswdigit(*(this->last_char));
         this->get_new_char())
    {
        if (result >= std::numeric_limits<uint32_t>::max() / 10)
            return std::nullopt;
        result *= 10;

        auto digit = convert_to_int(*(this->last_char));
        if (result >= std::numeric_limits<uint32_t>::max() - digit)
            return std::nullopt;
        result += digit;
    }
    return result;
}

std::optional<double> Lexer::parse_floating()
{
    double result = 0;
    for (double shift = 0.1;
         this->last_char.has_value() && std::iswdigit(*(this->last_char));
         this->get_new_char(), shift /= 10)
    {
        auto digit = convert_to_int(*(this->last_char));
        auto shifted_digit = digit * shift;
        result += shifted_digit;
    }
    return result;
}

std::optional<Token> Lexer::parse_number_token(const Position &position)
{
    auto integral = this->parse_integral();
    if (this->last_char == L'.')
    {
        this->get_new_char();
        auto floating = this->parse_floating();

        if (!floating.has_value())
            return this->report_and_consume(
                L"the floating part couldn't be parsed");

        if (integral.has_value())
            *floating += *integral;

        return Token(TokenType::DOUBLE, *floating, position);
    }
    if (!integral.has_value())
        return this->report_and_consume(
            L"the integral part exceeds the u32 limit");

    return Token(TokenType::INT, *integral, position);
}

std::optional<Token> Lexer::parse_operator(const Position &position)
{
    auto node = this->char_nodes.at(*(this->last_char));
    this->get_new_char();
    for (auto next_char = this->last_char;;
         this->get_new_char(), next_char = this->last_char)
    {
        decltype(node.children)::iterator child_iter;
        if (next_char.has_value() &&
            (child_iter = node.children.find(*(next_char))) !=
                node.children.end())
            node = child_iter->second;

        else
        {
            if (node.type.has_value())
                return Token(*node.type, position);
            else
                return this->report_and_consume(
                    L"this operator is not supported");
        }
    }
    // return std::nullopt;
}

Token Lexer::parse_comment_or_operator(const Position &position)
{
    this->get_new_char();
    if (this->last_char.has_value())
    {
        auto char_value = *(this->last_char);
        this->get_new_char();
        switch (char_value)
        {
        case L'/':
            return this->parse_line_comment(position);
            break;
        case L'*':
            return this->parse_block_comment(position);
            break;
        case L'=':
            return Token(TokenType::ASSIGN_SLASH, position);
            break;
        }
    }
    return Token(TokenType::SLASH, position);
}

Token Lexer::parse_line_comment(const Position &position)
{
    while (this->last_char.has_value() && this->last_char.value() != L'\n')
    {
        this->get_new_char();
    }
    return Token(TokenType::COMMENT, position);
}

Token Lexer::parse_block_comment(const Position &position)
{
    for (; this->last_char.has_value(); this->get_new_char())
    {
        if (this->last_char == L'*')
        {
            this->get_new_char();
            if (this->last_char == L'/')
            {
                this->get_new_char();
                break;
            }
        }
    }
    return Token(TokenType::COMMENT, position);
}

LexerPtr Lexer::from_wstring(const std::wstring &source,
                             const unsigned long long &max_var_name_size,
                             const unsigned long long &max_str_length)
{
    ReaderPtr reader = std::make_unique<StringReader>(source);
    return std::make_unique<Lexer>(std::move(reader), max_var_name_size,
                                   max_str_length);
}

LexerPtr Lexer::from_wstring(const std::wstring &source)
{
    ReaderPtr reader = std::make_unique<StringReader>(source);
    return std::make_unique<Lexer>(std::move(reader));
}

LexerPtr Lexer::from_file(const std::string &path,
                          const unsigned long long &max_var_name_size,
                          const unsigned long long &max_str_length)
{
    ReaderPtr reader = std::make_unique<FileReader>(path);
    return std::make_unique<Lexer>(std::move(reader), max_var_name_size,
                                   max_str_length);
}

LexerPtr Lexer::from_file(const std::string &path)
{
    ReaderPtr reader = std::make_unique<FileReader>(path);
    return std::make_unique<Lexer>(std::move(reader));
}

std::optional<wchar_t> Lexer::parse_hex_escape_sequence()
{
    this->get_new_char();
    std::wstring buffer = L"";

    for (int i = 0; i < 8 && this->last_char; ++i)
    {
        if (std::iswxdigit(*(this->last_char)))
        {
            buffer += *(this->last_char);
            this->get_new_char();
        }
        else if (this->last_char == L'}' && i == 0)
            return std::nullopt;
        else
            break;
    }

    if (this->last_char != L'}')
        return std::nullopt;
    this->get_new_char();
    if (buffer.empty())
        return std::nullopt;
    return static_cast<wchar_t>(std::stoi(buffer, nullptr, 16));
}

std::optional<wchar_t> Lexer::parse_escape_sequence()
{
    this->get_new_char();
    if (!this->last_char)
    {
        return std::nullopt;
    }
    wchar_t result;
    switch (*(this->last_char))
    {
    case L'\\':
        result = L'\\';
        break;
    case L'n':
        result = L'\n';
        break;
    case L'r':
        result = L'\r';
        break;
    case L't':
        result = L'\t';
        break;
    case L'\'':
        result = L'\'';
        break;
    case L'\"':
        result = L'\"';
        break;
    case L'0':
        result = L'\0';
        break;
    case L'{':
        return this->parse_hex_escape_sequence();
        break;

    default:
        return std::nullopt;
        break;
    }
    this->get_new_char();
    return result;
}

std::optional<wchar_t> Lexer::parse_language_char()
{
    if (!this->last_char.has_value())
        return std::nullopt;
    switch (*(this->last_char))
    {
    case '\\':
        return this->parse_escape_sequence();
    case '\'':
    case '\"':
        return std::nullopt;

    default:
        auto result = *(this->last_char);
        this->get_new_char();
        return result;
    }
}

Token Lexer::parse_char(const Position &position)
{
    this->get_new_char();
    wchar_t value = '\0';

    if (this->last_char == L'\"')
    {
        this->get_new_char();
        value = L'\"';
    }
    else if (auto opt_char = this->parse_language_char())
        value = *opt_char;

    if (this->last_char != L'\'')
        return this->report_and_consume(L"invalid char in a char literal");

    this->get_new_char();
    return Token(TokenType::CHAR, value, position);
}

Token Lexer::parse_str(const Position &position)
{
    this->get_new_char();
    std::wstringstream out_stream(L"");
    for (unsigned long long i = 0; i <= this->max_str_length; ++i)
    {
        if (this->last_char == L'\'')
        {
            this->get_new_char();
            out_stream << L'\'';
        }
        else if (auto opt_char = this->parse_language_char())
            out_stream << *opt_char;
        else
            break;
    }
    if (this->last_char != L'\"')
        return this->report_and_consume(L"str literal isn't enclosed");

    this->get_new_char();
    return Token(TokenType::STRING, out_stream.str(), position);
}

std::optional<Token> Lexer::get_token()
{
    this->get_nonempty_char();
    if (this->last_char == std::nullopt)
        return std::nullopt;
    else
    {
        auto position = this->position;
        switch (*(this->last_char))
        {
        case L'_':
            return this->parse_alpha_or_placeholder(position);
            break;
        case L'/':
            return this->parse_comment_or_operator(position);
            break;
        case L'\'':
            return this->parse_char(position);
            break;
        case L'\"':
            return this->parse_str(position);
            break;

        default:
            if (this->is_a_number_char())
                return this->parse_number_token(position);
            else if (this->is_identifier_char())
                return this->parse_alpha_or_placeholder(position);
            else if (this->is_an_operator_char())
                return this->parse_operator(position);
            else
                return this->report_and_consume(L"invalid char");

            break;
        }
    }
}

// the Lexer::is_*() functions below are called with an assumption, that
// the last_char optional has a value

bool Lexer::is_a_number_char() const
{
    auto character = *(this->last_char);
    return std::iswdigit(character);
}

bool Lexer::is_identifier_char() const
{
    auto character = *(this->last_char);
    return std::iswalpha(character) || character == L'_';
}

bool Lexer::is_an_operator_char() const
{
    return this->char_nodes.contains(*(this->last_char));
}

bool Lexer::is_alpha_char() const
{
    return this->last_char.has_value() &&
           (std::iswalnum(*(this->last_char)) || this->last_char == L'_');
}