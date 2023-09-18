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
    std::optional<TokenType> type;
    std::map<wchar_t, CharNode> children;
};
class Lexer;
using LexerPtr = std::unique_ptr<Lexer>;

class Lexer
{
    ReaderPtr reader;
    std::optional<wchar_t> last_char;
    std::string error_msg;
    std::locale locale;
    Position token_position;

    static const std::map<std::wstring, Token> keywords;
    static const std::map<wchar_t, CharNode> char_nodes;

    std::optional<wchar_t> get_new_char();
    std::optional<wchar_t> get_nonempty_char();

    Token parse_alpha_token();

    std::string parse_digits();
    Token parse_floating_remainder(std::string &num_str);
    Token parse_number_token();

    Token parse_operator();

    std::optional<Token> parse_possible_slash_token();
    Token parse_slash();

    Token parse_underscore();

    void skip_line_comment();
    void skip_block_comment();

    bool is_a_number_char() const;
    bool is_identifier_char() const;
    bool is_an_operator_char() const;

    std::optional<wchar_t> peek_char() const;
    Token report_error(const std::wstring &msg);

  public:
    Lexer(ReaderPtr &reader, const std::locale &locale)
        : reader(std::move(reader)), last_char(L' '), error_msg(""),
          locale(locale), token_position()
    {
    }

    Lexer(ReaderPtr &reader) : Lexer(reader, std::locale())
    {
    }

    static LexerPtr from_wstring(const std::wstring &source);
    static LexerPtr from_file(const std::string &path);

    Token get_token();

    const Position &get_position() const;
    bool eof() const;
};

#endif