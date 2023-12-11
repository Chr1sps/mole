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

class Parser
{
    static std::map<TokenType, BinOpData> binary_map;
    static std::map<TokenType, UnaryOpEnum> unary_map;
    static std::map<TokenType, TypeEnum> type_map;
    static std::map<TokenType, TypeEnum> type_value_map;
    static std::map<TokenType, AssignType> assign_map;

    LexerPtr lexer;
    std::optional<Token> current_token;
    std::unordered_set<Logger *> loggers;

    void next_token();

    void assert_current_and_eat(TokenType type, const std::wstring &error_msg);
    void assert_next_token(TokenType type, const std::wstring &error_msg);

    // type names

    TypePtr parse_type();
    TypePtr parse_simple_type();
    std::vector<TypePtr> parse_types();
    TypePtr parse_return_type();
    std::unique_ptr<FunctionType> parse_function_type();

    // function parameters

    ParamPtr parse_parameter();
    std::vector<ParamPtr> parse_params();

    // statements

    std::unique_ptr<Block> parse_else_block();

    TypePtr parse_type_specifier();
    ExprNodePtr parse_initial_value();
    std::unique_ptr<ReturnStmt> parse_return_stmt();
    std::unique_ptr<AssignStmt> parse_assign_statement();
    std::unique_ptr<Statement> parse_non_func_stmt();
    std::unique_ptr<Block> parse_block();
    std::unique_ptr<ContinueStmt> parse_continue_stmt();
    std::unique_ptr<BreakStmt> parse_break_stmt();
    std::unique_ptr<WhileStmt> parse_while_stmt();
    std::unique_ptr<IfStmt> parse_if_stmt();
    std::unique_ptr<MatchStmt> parse_match_stmt();
    std::unique_ptr<VarDeclStmt> parse_var_decl_stmt();
    std::unique_ptr<FuncDefStmt> parse_func_def_stmt();
    std::unique_ptr<ExternStmt> parse_extern_stmt();

    // helper methods

    std::tuple<std::wstring, std::vector<ParamPtr>, TypePtr>
    parse_func_name_and_params();

    std::unique_ptr<MatchArm> parse_match_arm();

    std::unique_ptr<LiteralArm> parse_literal_arm();
    std::unique_ptr<GuardArm> parse_guard_arm();
    std::unique_ptr<PlaceholderArm> parse_placeholder_arm();

    std::optional<std::tuple<Position, std::vector<ExprNodePtr> &&>>
    parse_literal_condition();

    ExprNodePtr parse_guard_condition();

    BlockPtr parse_match_arm_block();

    // expressions

    std::unique_ptr<VariableExpr> parse_variable_expr();

    std::unique_ptr<I32Expr> parse_u32_expr();
    std::unique_ptr<F64Expr> parse_f64_expr();
    std::unique_ptr<StringExpr> parse_string_expr();
    std::unique_ptr<CharExpr> parse_char_expr();
    std::unique_ptr<BoolExpr> parse_bool_expr();

    ExprNodePtr parse_paren_expr();

    ExprNodePtr parse_factor();
    ExprNodePtr parse_index_lambda_or_call();
    ExprNodePtr parse_unary_expr();
    ExprNodePtr parse_cast_expr();
    ExprNodePtr parse_binary_expr();

    std::optional<std::vector<ExprNodePtr>> parse_call_part();
    std::vector<ExprNodePtr> parse_args();

    std::optional<std::vector<ExprNodePtr>> parse_lambda_call_part();
    std::vector<ExprNodePtr> parse_lambda_args();
    std::optional<ExprNodePtr> parse_lambda_arg();

    ExprNodePtr parse_index_part();

    void report_error(const std::wstring &error_msg);

  public:
    Parser(LexerPtr &lexer) : lexer(std::move(lexer))
    {
        this->next_token();
    }

    Parser(LexerPtr &&lexer) : lexer(std::move(lexer))
    {
        this->next_token();
    }

    ProgramPtr parse();

    void add_logger(Logger *logger);
    void remove_logger(Logger *logger);
};

#endif