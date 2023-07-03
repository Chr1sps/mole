#ifndef __TOKEN_HPP__
#define __TOKEN_HPP__
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

    ASSIGN,       // '='
    LAMBDA_ARROW, // '=>'
    EQUAL,        // '=='

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

    PLACEHOLDER, // '_'
    ELLIPSIS,    // "..."

    KW_FN,     // "fn"
    KW_EXTERN, // "extern"
    KW_LET,    // "let"
    KW_RETURN, // "return"
    KW_MAIN,   // "main"
    KW_MUT,    // "mut"
    KW_CONST,  // "const"
    KW_IF,     // "if"
    KW_ELSE,   // "else"

    INT,
    UINT,
    FLOAT,
    DOUBLE,

    TYPE_BOOL, // "bool"

    TYPE_U8,  // "u8"
    TYPE_U16, // "u16"
    TYPE_U32, // "u32"
    TYPE_U64, // "u64"

    TYPE_I8,  // "i8"
    TYPE_I16, // "i16"
    TYPE_I32, // "i32"
    TYPE_I64, // "i64"

    TYPE_F32, // "f32"
    TYPE_F64, // "f64"

    TYPE_CHAR, // "char"

    END,

    OTHER,
    INVALID,
};

struct Token
{
    TokenType type;
    std::variant<std::wstring, double, int> value;
    Token() = default;

    Token(const TokenType &type) : type(type), value(0)
    {
    }

    Token(const TokenType &type, const int &num) : type(type), value(num)
    {
    }

    Token(const TokenType &type, const double &num) : type(type), value(num)
    {
    }

    Token(const TokenType &type, const std::wstring &str)
        : type(type), value(str)
    {
    }

    friend bool operator==(const Token &token, const Token &other);
    friend bool operator!=(const Token &token, const Token &other);
    friend bool operator==(const Token &token, const TokenType &type);
    friend bool operator!=(const Token &token, const TokenType &type);
};
#endif