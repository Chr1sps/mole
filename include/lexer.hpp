#ifndef __LEXER_HPP__
#define __LEXER_HPP__
#include "reader.hpp"
#include "token.hpp"
#include "locale.hpp"
#include <map>
#include <memory>
#include <string>
#include <locale>

class Lexer;
using LexerPtr = std::unique_ptr<Lexer>;
class Lexer
{
    ReaderPtr reader;
    wchar_t last_char;
    std::string error_msg;
    std::locale locale;
    std::map<std::wstring, Token> keywords{
        {L"fn", Token(TokenType::KW_FN)},
        {L"main", Token(TokenType::KW_MAIN)},
        {L"extern", Token(TokenType::KW_EXTERN)},
        {L"mut", Token(TokenType::KW_MUT)},
        {L"const", Token(TokenType::KW_CONST)},
        {L"let", Token(TokenType::KW_LET)},
        {L"return", Token(TokenType::KW_RETURN)},
        {L"if", Token(TokenType::KW_IF)},
        {L"else", Token(TokenType::KW_ELSE)},
    };

    wchar_t get_new_char();
    void get_nonempty_char();
    Token parse_alpha_token();
    Token parse_number_token();
    // void skip_comment();

public:
    Lexer(ReaderPtr &reader) : reader(std::move(reader)),
                               last_char(L' '),
                               error_msg(""),
                               locale(Locale::get().locale()) {}

    static LexerPtr from_wstring(const std::wstring &source);

    Token get_token();
    wchar_t peek_char();
    bool eof();
};
#endif