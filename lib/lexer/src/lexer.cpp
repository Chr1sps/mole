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
    CHAR_NODE(L'@', AT),
};

#undef CHAR_NODE
#undef EMPTY_NODE

std::optional<IndexedChar> Lexer::get_new_char()
{
    return this->last_char = this->reader->get();
}

std::optional<IndexedChar> Lexer::get_nonempty_char()
{
    while (this->last_char.has_value() &&
           std::isspace(this->last_char->character, this->locale))
    {
        this->get_new_char();
    }
    return this->last_char;
}

Token Lexer::parse_alpha_token(const Position &position)
{
    std::wstring name;
    do
    {
        name += this->last_char->character;
        this->get_new_char();
    } while (this->last_char &&
             (std::isalnum(this->last_char->character, this->locale) ||
              this->last_char == L'_') &&
             name.length() <= Lexer::MAX_VAR_NAME_SIZE);
    if (auto name_iter = this->keywords.find(name);
        name_iter != this->keywords.end())
        return Token(name_iter->second, position);
    return Token(TokenType::IDENTIFIER, name, position);
}

std::string Lexer::parse_digits()
{
    std::string num_str;
    while (this->last_char.has_value() &&
           std::isdigit(this->last_char->character, this->locale))
    {
        num_str += this->last_char->character;
        this->get_new_char();
    }
    return num_str;
}

Token Lexer::parse_floating_remainder(std::string &num_str,
                                      const Position &position)
{
    num_str += '.';
    this->get_new_char();
    num_str += this->parse_digits();
    double value = std::strtod(num_str.c_str(), 0);
    return Token(TokenType::DOUBLE, value, position);
}

Token Lexer::parse_number_token(const Position &position)
{
    std::string num_str;
    num_str += this->parse_digits();
    if (this->last_char.has_value() && *this->last_char == L'.')
    {
        return this->parse_floating_remainder(num_str, position);
    }
    else
    {
        auto value = std::stoull(num_str.c_str(), 0);
        return Token(TokenType::INT, value, position);
    }
}

std::optional<Token> Lexer::parse_operator(const Position &position)
{
    auto node = this->char_nodes.at(this->last_char->character);
    TokenType return_type;
    std::optional<Token> result;
    for (auto next_char = this->peek_char();;
         this->get_new_char(), next_char = this->peek_char())
    {
        if (decltype(node.children)::iterator child_iter;
            next_char.has_value() &&
            (child_iter = node.children.find(next_char->character)) !=
                node.children.end())
            node = child_iter->second;

        else
        {
            try
            {
                return_type = node.type.value();
                result = Token(return_type, position);
            }
            catch (const std::bad_optional_access &e)
            {
                result = this->report_error(L"this operator is not supported");
            }
            this->get_new_char();
            break;
        }
    }
    return result;
}

std::optional<Token> Lexer::parse_possible_slash_token(
    const Position &position)
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
                Token(TokenType::ASSIGN_SLASH, position));
        }
    }
    return std::optional<Token>(Token(TokenType::SLASH, position));
}

std::optional<Token> Lexer::parse_slash(const Position &position)
{
    auto result = this->parse_possible_slash_token(position);
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

Token Lexer::parse_underscore(const Position &position)
{
    auto next = this->reader->peek();
    if (next.has_value() && std::isalnum(next->character, this->locale))
    {
        return this->parse_alpha_token(position);
    }
    else
    {
        this->get_new_char();
        return Token(TokenType::PLACEHOLDER, position);
    }
}

std::optional<wchar_t> Lexer::parse_hex_escape_sequence()
{
    this->get_new_char();
    std::wstring buffer = L"";

    for (int i = 0; i < 8 && this->last_char; ++i)
    {
        if (std::iswxdigit(this->last_char->character))
        {
            buffer += this->last_char->character;
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
    switch (this->last_char->character)
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
    switch (this->last_char->character)
    {
    case '\\':
        return this->parse_escape_sequence();
    case '\'':
    case '\"':
        return std::nullopt;

    default:
        auto result = this->last_char->character;
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
        return this->report_error(L"invalid char in a char literal");
    this->get_new_char();
    return Token(TokenType::CHAR, value, position);
}

Token Lexer::parse_str(const Position &position)
{
    this->get_new_char();
    std::wstringstream out_stream(L"");
    for (unsigned long long i = 0; i <= Lexer::MAX_STR_SIZE; ++i)
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
        return this->report_error(L"str literal isn't enclosed");
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
        auto position = this->last_char->position;
        switch (this->last_char->character)
        {
        case L'_':
            return this->parse_underscore(position);
            break;
        case L'/':
            return this->parse_slash(position);
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
                return this->parse_alpha_token(position);
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
    auto character = this->last_char.value().character;
    auto next_char = this->peek_char();
    return std::isdigit(character, this->locale) ||
           (character == L'.' && next_char.has_value() &&
            std::isdigit(next_char.value().character, this->locale));
}

bool Lexer::is_identifier_char() const
{
    auto character = this->last_char.value().character;
    return std::isalpha(character, this->locale) || character == L'_';
}

bool Lexer::is_an_operator_char() const
{
    return this->char_nodes.contains(this->last_char.value().character);
}

std::optional<IndexedChar> Lexer::peek_char() const
{
    return this->reader->peek();
}

Token Lexer::report_error(const std::wstring &msg)
{
    std::wstring position_text;
    Position position = Position(-1, -1);
    if (this->last_char)
    {
        position = this->last_char->position;
        position_text = build_wstring(position.line, ",", position.column);
    }
    else
    {
        position_text = L"EOF";
    }
    auto error_text = build_wstring(L"[ERROR] Lexer error at [", position_text,
                                    "]: ", msg, ".");
    auto log_msg = LogMessage(error_text, LogLevel::ERROR);
    for (auto logger : this->loggers)
    {
        logger->log(log_msg);
    }

    return Token(TokenType::INVALID, position);
}

Token Lexer::report_and_consume(const std::wstring &msg)
{
    auto result = this->report_error(msg);
    this->get_new_char();
    return result;
}

void Lexer::add_logger(const LoggerPtr &logger)
{
    this->loggers.push_back(logger);
}

void Lexer::remove_logger(const LoggerPtr &logger)
{
    std::erase(this->loggers, logger);
}