#ifndef __TOKEN_HPP__
#define __TOKEN_HPP__
#include <string>
#include <variant>
enum class TokenType
{
    IDENTIFIER,
    INT,
    F32,
    F64,
    END,

    COLON,     // ':'
    COMMA,     // ','
    SEMICOLON, // ';'

    PLUS,        // '+'
    INCREMENT,   // '++'
    ASSIGN_PLUS, // '+='

    MINUS,        // '-'
    DECREMENT,    // '--'
    ASSIGN_MINUS, // '-='

    STAR,        // '*'
    ASSIGN_STAR, // '*='

    SLASH,        // '/'
    ASSIGN_SLASH, // '/='

    PERCENT,        // '%'
    ASSIGN_PERCENT, // '%='

    BIT_NEG,        // '~'
    ASSIGN_BIT_NEG, // '~='

    ASSIGN,       // '='
    LAMBDA_ARROW, // '=>'
    EQUAL,        // '=='

    LESS,       // '<'
    LESS_EQUAL, // '<='

    GREATER,       // '>'
    GREATER_EQUAL, // '>='

    NEG,       // '!'
    NOT_EQUAL, // '!='

    BIT_XOR,        // ^
    ASSIGN_BIT_XOR, // ^=
    EXP,            // '^^'

    AMPERSAND,        // '&'
    ASSIGN_AMPERSAND, // '&='
    AND,              // '&&'

    BIT_OR,        // '|'
    ASSIGN_BIT_OR, // '|='
    OR,            // '||'

    L_BRACKET,    // '{'
    R_BRACKET,    // '}'
    L_PAREN,      // '('
    R_PAREN,      // ')'
    L_SQ_BRACKET, // '['
    R_SQ_BRACKET, // ']'

    KW_FN,     // "fn"
    KW_MAIN,   // "main"
    KW_EXTERN, // "extern"
    KW_MUT,    // "mut"
    KW_CONST,  // "const"
    KW_LET,    // "let"
    KW_RETURN, // "return"
    KW_IF,     // "if"
    KW_ELSE,   // "else"

    OTHER,
};
struct Token
{
    TokenType type;
    std::variant<std::wstring, double, int> value;
    Token() = default;
    Token(const TokenType &type)
        : type(type), value(0) {}
    Token(const TokenType &type, const int &num)
        : type(type), value(num) {}
    Token(const TokenType &type, const double &num)
        : type(type), value(num) {}
    Token(const TokenType &type, const std::wstring &str)
        : type(type), value(str) {}
    friend bool operator==(const Token &token, const Token &other);
    friend bool operator!=(const Token &token, const Token &other);
};
#endif