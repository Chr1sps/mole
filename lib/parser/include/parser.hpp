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
    static std::map<TokenType, AssignType> assign_map;

    LexerPtr lexer;
    std::optional<Token> current_token;

    void next_token();

    bool assert_current_and_eat(TokenType type, const std::wstring &error_msg);

    // type names

    TypePtr parse_type();
    TypePtr parse_simple_type();
    std::vector<TypePtr> parse_types();
    TypePtr parse_return_type();
    TypePtr parse_function_type();

    // function parameters

    ParamPtr parse_parameter();
    std::vector<ParamPtr> parse_params();

    // statements

    StmtPtr parse_else_block();

    TypePtr parse_type_specifier();
    ExprNodePtr parse_initial_value();

    std::optional<AssignType> parse_assign_op();

    std::optional<std::tuple<AssignType, ExprNodePtr>> parse_assign_part();

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
    std::unique_ptr<FuncDefStmt> parse_func_def_stmt();
    std::unique_ptr<ExternStmt> parse_extern_stmt();

    // helper methods

    std::optional<std::tuple<std::wstring, std::vector<ParamPtr>, TypePtr>>
    parse_func_name_and_params();

    MatchArmPtr parse_match_arm();

    MatchArmPtr parse_literal_arm();
    MatchArmPtr parse_guard_arm();
    MatchArmPtr parse_else_arm();

    std::optional<std::tuple<Position, std::vector<ExprNodePtr>>>
    parse_literal_condition();

    std::optional<std::tuple<Position, ExprNodePtr>> parse_guard_condition();

    StmtPtr parse_match_arm_block();

    // expressions

    ExprNodePtr parse_u32_expr();
    ExprNodePtr parse_f64_expr();
    ExprNodePtr parse_string_expr();
    ExprNodePtr parse_char_expr();
    ExprNodePtr parse_bool_expr();
    ExprNodePtr parse_variable_expr();

    ExprNodePtr parse_paren_expr();

    ExprNodePtr parse_factor();
    ExprNodePtr parse_index_lambda_or_call();
    ExprNodePtr parse_unary_expr();
    ExprNodePtr parse_cast_expr();
    ExprNodePtr parse_binary_expr();

    std::optional<std::vector<ExprNodePtr>> parse_call_part();
    std::vector<ExprNodePtr> parse_args();

    std::optional<std::vector<ExprNodePtr>> parse_lambda_call_part();
    std::optional<std::vector<ExprNodePtr>> parse_lambda_args();
    std::optional<ExprNodePtr> parse_lambda_arg();

    ExprNodePtr parse_index_part();

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

#endif