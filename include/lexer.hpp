#ifndef __LEXER_HPP__
#define __LEXER_HPP__
#include "reader.hpp"
#include "token.hpp"
#include <memory>

class Lexer
{
    ReaderPtr reader;
    wchar_t last_char;

    wchar_t get_new_char();
    void get_nonempty_char();
    // Token parse_alpha_token();
    // Token parse_number_token();
    // void skip_comment();

public:
    Lexer(ReaderPtr &reader) : reader(std::move(reader)), last_char(' ') {}

    Token get_token();
    wchar_t peek_char();
};
using LexerPtr = std::unique_ptr<Lexer>;
#endif