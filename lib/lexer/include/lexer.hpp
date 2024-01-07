#ifndef __LEXER_HPP__
#define __LEXER_HPP__
#include "logger.hpp"
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
    std::optional<TokenType> type;
    std::map<wchar_t, CharNode> children;
};
class Lexer;
using LexerPtr = std::unique_ptr<Lexer>;

class Lexer : public Reporter
{
    static const std::map<std::wstring, TokenType> keywords;
    static const std::map<wchar_t, CharNode> char_nodes;

    ReaderPtr reader;
    const unsigned long long max_var_name_size;
    const unsigned long long max_str_length;

    Position position;
    std::optional<wchar_t> last_char;
    std::vector<Logger *> loggers;

    Token report_and_throw(const std::wstring &msg);

    std::optional<wchar_t> get_new_char();
    std::optional<wchar_t> get_nonempty_char();

    std::optional<unsigned long long> parse_integral();
    std::optional<double> parse_floating();

    std::optional<Token> parse_number_token(const Position &position);

    std::optional<wchar_t> parse_hex_escape_sequence();
    std::optional<Token> parse_alpha_or_placeholder(const Position &position);
    Token parse_comment_or_operator(const Position &position);
    std::optional<Token> parse_operator(const Position &position);
    Token parse_char(const Position &position);
    Token parse_str(const Position &position);

    std::optional<wchar_t> parse_escape_sequence();
    std::optional<wchar_t> parse_language_char();

    Token parse_line_comment(const Position &position);
    Token parse_block_comment(const Position &position);

    bool is_a_number_char() const;
    bool is_identifier_char() const;
    bool is_an_operator_char() const;
    bool is_alpha_char() const;

  public:
    Lexer(ReaderPtr reader, const unsigned long long &max_var_name_size,
          const unsigned long long &max_str_length)
        : reader(std::move(reader)), max_var_name_size(max_var_name_size),
          max_str_length(max_str_length), position(0, 0)
    {
        // this loads the first character into the lexer so that when we run
        // the get_nonempty_char() function at the very first get_token() call
        // it doesn't immediately return a nullopt and stop the lexer
        this->get_new_char();
    }

    Lexer(ReaderPtr reader)
        : Lexer(std::move(reader), (1 << 8) - 1, (1 << 16) - 1)
    {
    }

    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = default;
    Lexer &operator=(const Lexer &) = delete;
    Lexer &operator=(Lexer &&) = delete;

    std::optional<Token> get_token();

    static LexerPtr from_wstring(const std::wstring &source);
    static LexerPtr from_wstring(const std::wstring &source,
                                 const unsigned long long &max_var_name_size,
                                 const unsigned long long &max_str_length);
    static LexerPtr from_file(const std::string &path);
    static LexerPtr from_file(const std::string &path,
                              const unsigned long long &max_var_name_size,
                              const unsigned long long &max_str_length);
};

class LexerException : public std::runtime_error
{
  public:
    LexerException() : std::runtime_error("Lexer error.")
    {
    }
};

#endif