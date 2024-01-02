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

    BIT_NEG, // '~'

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

    AT,           // '@'
    PLACEHOLDER,  // '_'
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
    KW_TRUE,     // "true"
    KW_FALSE,    // "false"

    TYPE_BOOL, // "bool"

    TYPE_U32, // "u32"
    TYPE_I32, // "i32"
    TYPE_F64, // "f64"

    TYPE_CHAR, // "char"
    TYPE_STR,  // "str"

    INT,    // holds a u32 value
    DOUBLE, // holds a f64 value
    STRING, // holds a str value
    CHAR,   // holds a char value

    COMMENT, // represents a comment, doesn't hold the string of the comment

    // Token used as a placeholder for invalid values; it is used as a means of
    // distinguishing between when a lexer output an invalid value (by
    // returning an invalid token) and when lexer has no more tokens to output
    // (by returning std::nullopt).
    INVALID
};

struct Token
{
    TokenType type;
    std::variant<std::wstring, wchar_t, double, unsigned long long> value;
    Position position;

    Token(const TokenType &type, const Position &position)
        : type(type), value(0ull), position(position)
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

    Token(const TokenType &type, const wchar_t &chr, const Position &position)
        : type(type), value(chr), position(position)
    {
    }

    friend bool operator==(const Token &token, const Token &other);
    friend bool operator!=(const Token &token, const Token &other);
    friend bool operator==(const Token &token, const TokenType &type);
    friend bool operator!=(const Token &token, const TokenType &type);
};
#endif