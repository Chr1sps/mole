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

    PLUS,    // '+'
    MINUS,   // '-'
    STAR,    // '*'
    SLASH,   // '/'
    PERCENT, // '%'
    EXP,     // '^^'

    ASSIGN,        // '='
    EQUAL,         // '=='
    NOT_EQUAL,     // '!='
    LESS,          // '<'
    GREATER,       // '>'
    LESS_EQUAL,    // '<='
    GREATER_EQUAL, // '>='

    AMPERSAND, // '&'
    BIT_OR,    // '|'
    BIT_XOR,   // ^
    BIT_NEG,   // '~'
    OR,        // '||'
    AND,       // '&&'
    NEG,       // '!'

    L_BRACKET,    // '{'
    R_BRACKET,    // '}'
    L_PAREN,      // '('
    R_PAREN,      // ')'
    L_SQ_BRACKET, // '['
    R_SQ_BRACKET, // ']'

    COLON,     // ':'
    COMMA,     // ','
    SEMICOLON, // ';'
    PIPE,      // '|'

    LAMBDA_ARROW, // '=>'

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