#ifndef __TOKEN_HPP__
#define __TOKEN_HPP__
#include "position.hpp"
#include <string>
#include <variant>
enum class TokenType
{
    IDENTIFIER,

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

    ASSIGN, // '='
    EQUAL,  // '=='

    LESS,              // '<'
    LESS_EQUAL,        // '<='
    SHIFT_LEFT,        // "<<"
    ASSIGN_SHIFT_LEFT, // "<<="

    GREATER,            // '>'
    GREATER_EQUAL,      // '>='
    SHIFT_RIGHT,        // ">>"
    ASSIGN_SHIFT_RIGHT, // ">>="

    NEG,       // '!'
    NOT_EQUAL, // '!='

    BIT_XOR,        // ^
    ASSIGN_BIT_XOR, // ^=
    EXP,            // '^^'
    ASSIGN_EXP,     // "^^="

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

    PLACEHOLDER,  // '_'
    ELLIPSIS,     // "..."
    LAMBDA_ARROW, // '=>'

    KW_FN,       // "fn"
    KW_EXTERN,   // "extern"
    KW_LET,      // "let"
    KW_RETURN,   // "return"
    KW_MUT,      // "mut"
    KW_CONST,    // "const"
    KW_IF,       // "if"
    KW_ELSE,     // "else"
    KW_WHILE,    // "while"
    KW_MATCH,    // "match"
    KW_CONTINUE, // "continue"
    KW_BREAK,    // "break"
    KW_AS,       // "as"

    INT,    // holds a u32 value
    DOUBLE, // holds a f64 value
    STRING, // holds a str value
    CHAR,   // holds a char value

    TYPE_BOOL, // "bool"

    TYPE_U32, // "u32"
    TYPE_I32, // "i32"
    TYPE_F64, // "f64"

    TYPE_CHAR, // "char"
    TYPE_STR,  // "str"
};

struct Token
{
    TokenType type;
    std::variant<std::wstring, double, long long, unsigned long long> value;
    Position position;
    Token() = default;

    Token(const TokenType &type, const Position &position)
        : type(type), value(0), position(position)
    {
    }

    Token(const TokenType &type, const long long &num,
          const Position &position)
        : type(type), value(num), position(position)
    {
    }

    Token(const TokenType &type, const unsigned long long &num,
          const Position &position)
        : type(type), value(num), position(position)
    {
    }

    Token(const TokenType &type, const double &num, const Position &position)
        : type(type), value(num), position(position)
    {
    }

    Token(const TokenType &type, const std::wstring &str,
          const Position &position)
        : type(type), value(str), position(position)
    {
    }

    friend bool operator==(const Token &token, const Token &other);
    friend bool operator!=(const Token &token, const Token &other);
    friend bool operator==(const Token &token, const TokenType &type);
    friend bool operator!=(const Token &token, const TokenType &type);
};
#endif