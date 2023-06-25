#ifndef __PARSER_HPP__
#define __PARSER_HPP__
#include "lexer.hpp"

class Parser
{
    LexerPtr lexer;
    Token current_token;

  public:
    Parser(LexerPtr &lexer) : lexer(std::move(lexer))
    {
    }
};
#endif