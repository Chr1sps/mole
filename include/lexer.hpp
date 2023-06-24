#ifndef __LEXER_HPP__
#define __LEXER_HPP__
#include "locale.hpp"
#include "reader.hpp"
#include "token.hpp"
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct CharNode
{
    TokenType type;
    std::map<wchar_t, CharNode> children;
};
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

        // type names
        {L"f32", Token(TokenType::TYPE_F32)},
        {L"f64", Token(TokenType::TYPE_F64)},

        {L"u8", Token(TokenType::TYPE_U8)},
        {L"u16", Token(TokenType::TYPE_U16)},
        {L"u32", Token(TokenType::TYPE_U32)},
        {L"u64", Token(TokenType::TYPE_U64)},

        {L"i8", Token(TokenType::TYPE_I8)},
        {L"i16", Token(TokenType::TYPE_I16)},
        {L"i32", Token(TokenType::TYPE_I32)},
        {L"i64", Token(TokenType::TYPE_I64)},

        {L"char", Token(TokenType::TYPE_CHAR)},

    };
    std::map<wchar_t, CharNode> char_nodes{
        {L':', {TokenType::COLON, {}}},
        {L',', {TokenType::COMMA, {}}},
        {L';', {TokenType::SEMICOLON, {}}},
        {L'+',
         {TokenType::PLUS,
          {
              {L'+', {TokenType::INCREMENT, {}}},
              {L'=', {TokenType::ASSIGN_PLUS, {}}},
          }}},
        {L'-',
         {TokenType::MINUS,
          {
              {L'-', {TokenType::DECREMENT, {}}},
              {L'=', {TokenType::ASSIGN_MINUS, {}}},
          }}},
        {L'*',
         {TokenType::STAR,
          {
              {L'=', {TokenType::ASSIGN_STAR, {}}},
          }}},
        // {L'/', {TokenType::SLASH, {
        //                               {L'=', {TokenType::ASSIGN_SLASH, {}}},
        //                           }}},
        {L'%',
         {TokenType::PERCENT,
          {
              {L'=', {TokenType::ASSIGN_PERCENT, {}}},
          }}},
        {L'~',
         {TokenType::BIT_NEG,
          {
              {L'=', {TokenType::ASSIGN_BIT_NEG, {}}},
          }}},
        {L'=',
         {TokenType::ASSIGN,
          {
              {L'>', {TokenType::LAMBDA_ARROW, {}}},
              {L'=', {TokenType::EQUAL, {}}},
          }}},
        {L'<',
         {TokenType::LESS,
          {
              {L'=', {TokenType::LESS_EQUAL, {}}},
          }}},
        {L'>',
         {TokenType::GREATER,
          {
              {L'=', {TokenType::GREATER_EQUAL, {}}},
          }}},
        {L'!',
         {TokenType::NEG,
          {
              {L'=', {TokenType::NOT_EQUAL, {}}},
          }}},
        {L'^',
         {TokenType::BIT_XOR,
          {
              {L'=', {TokenType::ASSIGN_BIT_XOR, {}}},
              {L'^', {TokenType::EXP, {}}},
          }}},
        {L'&',
         {TokenType::AMPERSAND,
          {
              {L'=', {TokenType::ASSIGN_AMPERSAND, {}}},
              {L'&', {TokenType::AND, {}}},
          }}},
        {L'|',
         {TokenType::BIT_OR,
          {
              {L'=', {TokenType::ASSIGN_BIT_OR, {}}},
              {L'|', {TokenType::OR, {}}},
          }}},
        {L'{', {TokenType::L_BRACKET, {}}},
        {L'}', {TokenType::R_BRACKET, {}}},
        {L'(', {TokenType::L_PAREN, {}}},
        {L')', {TokenType::R_PAREN, {}}},
        {L'[', {TokenType::L_SQ_BRACKET, {}}},
        {L']', {TokenType::R_SQ_BRACKET, {}}},
    };

    wchar_t get_new_char();
    void get_nonempty_char();
    Token parse_alpha_token();
    std::string parse_digits();
    Token parse_number_token();
    Token parse_operator();
    std::optional<Token> parse_slash();
    void skip_line_comment();
    void skip_block_comment();

  public:
    Lexer(ReaderPtr &reader) : reader(std::move(reader)), last_char(L' '), error_msg(""), locale(Locale::get().locale())
    {
    }

    static LexerPtr from_wstring(const std::wstring &source);

    Token get_token();
    wchar_t peek_char();
    bool eof();
};
class LexerException : public std::exception
{
    const char *what_str;

  public:
    LexerException() = default;
    LexerException(const char *what_str) : what_str(what_str)
    {
    }
    const char *what() const noexcept override
    {
        return this->what_str;
    }
};
#endif