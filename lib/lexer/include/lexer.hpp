#ifndef __LEXER_HPP__
#define __LEXER_HPP__
#include "locale.hpp"
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

class Lexer
{
    ReaderPtr reader;

    std::optional<wchar_t> last_char;
    Position token_position;

    std::locale locale;

    std::vector<LoggerPtr> loggers;

    static const std::map<std::wstring, TokenType> keywords;
    static const std::map<wchar_t, CharNode> char_nodes;

    std::optional<wchar_t> get_new_char();
    std::optional<wchar_t> get_nonempty_char();

    Token parse_alpha_token();

    std::string parse_digits();
    Token parse_floating_remainder(std::string &num_str);
    Token parse_number_token();

    Token parse_operator();

    std::optional<Token> parse_possible_slash_token();
    std::optional<Token> parse_slash();

    Token parse_underscore();

    void skip_line_comment();
    void skip_block_comment();

    bool is_a_number_char() const;
    bool is_identifier_char() const;
    bool is_an_operator_char() const;

    std::optional<wchar_t> peek_char() const;
    Token report_error(const std::wstring &msg);

  public:
    Lexer(ReaderPtr &reader)
        : reader(std::move(reader)), last_char(L' '), token_position()
    {
        this->locale = this->reader->get_locale();
    }

    static LexerPtr from_wstring(const std::wstring &source);
    static LexerPtr from_file(const std::string &path);

    std::optional<Token> get_token();

    const Position &get_position() const;
    bool eof() const;
};

#endif