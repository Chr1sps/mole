#include "token.hpp"

bool operator==(const Token &token, const Token &other)
{
    return token.type == other.type && token.value == other.value;
}

bool operator!=(const Token &token, const Token &other)
{
    return !(token == other);
}

bool operator==(const Token &token, const TokenType &type)
{
    return token.type == type;
}

bool operator!=(const Token &token, const TokenType &type)
{
    return !(token == type);
}
