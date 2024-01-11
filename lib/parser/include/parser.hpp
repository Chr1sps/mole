#ifndef __PARSER_HPP__
#define __PARSER_HPP__
#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <map>
#include <unordered_set>

struct BinOpData
{
    BinOpEnum type;
    int precedence;
    bool is_right_assoc;

    BinOpData(const BinOpEnum &type, const int &precedence,
              const bool &is_right_assoc)
        : type(type), precedence(precedence), is_right_assoc(is_right_assoc)
    {
    }
};

class Parser : public Reporter
{
    static std::map<TokenType, BinOpData> binary_map;
    static std::map<TokenType, UnaryOpEnum> unary_map;
    static std::map<TokenType, TypeEnum> type_map;
    static std::map<TokenType, TypeEnum> type_value_map;
    static std::map<TokenType, std::optional<BinOpEnum>> assign_map;

    LexerPtr lexer;
    std::optional<Token> current_token;

    void next_token();

    bool assert_current_and_eat(TokenType type, const std::wstring &error_msg);

    // type names

    std::optional<Type> parse_type();
    std::optional<Type> parse_return_type();

    // function parameters

    ParamPtr parse_parameter();
    std::vector<ParamPtr> parse_params();

    // statements

    StmtPtr parse_else_block();

    std::optional<Type> parse_type_specifier();
    ExprPtr parse_initial_value();

    std::optional<std::optional<BinOpEnum>> parse_assign_op();

    std::optional<std::tuple<std::optional<BinOpEnum>, ExprPtr>>
    parse_assign_part();

    StmtPtr parse_return_stmt();
    StmtPtr parse_assign_or_expr_stmt();
    StmtPtr parse_non_func_stmt();
    StmtPtr parse_continue_stmt();
    StmtPtr parse_break_stmt();
    StmtPtr parse_while_stmt();
    StmtPtr parse_if_stmt();
    StmtPtr parse_match_stmt();

    std::unique_ptr<Block> parse_block();

    std::unique_ptr<VarDeclStmt> parse_var_decl_stmt();
    std::unique_ptr<FuncDef> parse_func_def_stmt();
    std::unique_ptr<ExternDef> parse_extern_stmt();

    // helper methods

    std::optional<
        std::tuple<std::wstring, std::vector<ParamPtr>, std::optional<Type>>>
    parse_func_name_and_params();

    MatchArmPtr parse_match_arm();

    MatchArmPtr parse_literal_arm();
    MatchArmPtr parse_guard_arm();
    MatchArmPtr parse_else_arm();

    std::optional<std::tuple<Position, std::vector<ExprPtr>>>
    parse_literal_condition();

    std::optional<std::tuple<Position, ExprPtr>> parse_guard_condition();

    StmtPtr parse_match_arm_block();

    std::optional<BinOpData> parse_binop();
    std::optional<std::tuple<UnaryOpEnum, Position>> parse_unop();

    // expressions

    ExprPtr parse_u32_expr();
    ExprPtr parse_f64_expr();
    ExprPtr parse_string_expr();
    ExprPtr parse_char_expr();
    ExprPtr parse_bool_expr();
    ExprPtr parse_variable_or_call();

    ExprPtr parse_paren_expr();

    ExprPtr parse_factor();
    ExprPtr parse_index_expr();
    ExprPtr parse_unary_expr();
    ExprPtr parse_cast_expr();
    ExprPtr parse_binary_expr();

    ExprPtr parse_call(const std::wstring &, const Position &);
    std::vector<ExprPtr> parse_args();

    ExprPtr parse_lambda_call(ExprPtr &&expr);
    std::optional<std::vector<ExprPtr>> parse_lambda_args();
    std::optional<ExprPtr> parse_lambda_arg();

    ExprPtr parse_index(ExprPtr &&expr);

    void report_error(const std::wstring &msg);

  public:
    Parser() noexcept : lexer(nullptr)
    {
    }

    Parser(LexerPtr lexer) noexcept : lexer(std::move(lexer))
    {
        this->next_token();
    }

    Parser(const Parser &) = delete;
    Parser(Parser &&) = default;
    Parser &operator=(const Parser &) = delete;
    Parser &operator=(Parser &&) = default;

    ProgramPtr parse();

    LexerPtr attach_lexer(LexerPtr &lexer) noexcept;
    LexerPtr detach_lexer() noexcept;
    bool is_lexer_attached() const noexcept;
};

class ParserException : public std::runtime_error
{
  public:
    ParserException() : std::runtime_error("Parser error.")
    {
    }
};

#endif