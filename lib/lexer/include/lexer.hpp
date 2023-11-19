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
    std::optional<IndexedChar> last_char;
    std::locale locale;
    std::vector<LoggerPtr> loggers;

    static const std::map<std::wstring, TokenType> keywords;
    static const std::map<wchar_t, CharNode> char_nodes;

    std::optional<IndexedChar> get_new_char();
    std::optional<IndexedChar> peek_char() const;

    std::optional<IndexedChar> get_nonempty_char();

    Token parse_number_token(const Position &position);
    Token parse_underscore(const Position &position);
    Token parse_alpha_token(const Position &position);
    std::optional<Token> parse_slash(const Position &position);
    std::optional<Token> parse_operator(const Position &position);

    std::string parse_digits();
    Token parse_floating_remainder(std::string &num_str,
                                   const Position &position);

    std::optional<Token> parse_possible_slash_token(const Position &position);

    void skip_line_comment();
    void skip_block_comment();

    bool is_a_number_char() const;
    bool is_identifier_char() const;
    bool is_an_operator_char() const;

    std::nullopt_t report_error(const std::wstring &msg);

  public:
    Lexer(ReaderPtr &reader) : reader(std::move(reader))
    {
        // getting the locale from the reader instead of loading it some other
        // way prevents locale mismatch
        this->locale = this->reader->get_locale();

        // this loads the first character into the lexer so that when we run
        // the get_nonempty_char() function at the very first get_token() call
        // it doesn't immediately return a nullopt and stop the lexer
        this->get_new_char();
    }

    static LexerPtr from_wstring(const std::wstring &source);
    static LexerPtr from_file(const std::string &path);

    std::optional<Token> get_token();

    void add_logger(const LoggerPtr &logger);
    void remove_logger(const LoggerPtr &logger);
};

#endif