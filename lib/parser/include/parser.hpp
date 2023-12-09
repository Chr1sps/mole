#ifndef __PARSER_HPP__
#define __PARSER_HPP__
#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <map>

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
    LexerPtr lexer;
    std::optional<Token> current_token;
    static std::map<TokenType, BinOpData> binary_map;
    static std::map<TokenType, UnaryOpEnum> unary_map;
    static std::map<TokenType, TypeEnum> type_map;
    static std::map<TokenType, TypeEnum> type_value_map;
    static std::map<TokenType, AssignType> assign_map;

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

    std::optional<TypePtr> parse_var_type();
    std::optional<ExprNodePtr> parse_var_value();
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

    std::vector<ExprNodePtr> parse_literal_condition();
    ExprNodePtr parse_literal_pattern();

    ExprNodePtr parse_guard_condition();

    std::unique_ptr<BlockPtr> parse_match_arm_block();

    void check_next_op_and_parse(std::unique_ptr<ExprNode> &lhs,
                                 std::unique_ptr<ExprNode> &rhs,
                                 const std::shared_ptr<BuiltInBinOp> &op);
    void push_expr(std::vector<ExprNodePtr> &args,
                   std::vector<std::optional<ExprNodePtr>> &lambda_args,
                   const bool &is_lambda);

    bool eat_comma_or_rparen();

    // expressions

    std::unique_ptr<I32Expr> parse_i32();
    std::unique_ptr<F64Expr> parse_f64();
    std::unique_ptr<ExprNode> parse_paren_expr();
    std::unique_ptr<ExprNode> parse_unary_expression();
    std::unique_ptr<ExprNode> parse_call_or_lambda(ExprNodePtr &expr);
    std::unique_ptr<ExprNode> parse_identifier_expression();
    std::unique_ptr<ExprNode> parse_const_expression();
    std::vector<std::unique_ptr<ExprNode>> parse_call_args();
    std::unique_ptr<ExprNode> parse_lhs();
    std::unique_ptr<ExprNode> parse_op_and_rhs(const unsigned &precedence,
                                               std::unique_ptr<ExprNode> &lhs);
    std::unique_ptr<ExprNode> parse_expression();

    void report_error(const std::wstring &error_msg);

  public:
    Parser(LexerPtr &lexer) : lexer(std::move(lexer)), current_token()
    {
    }

    Parser(LexerPtr &&lexer) : lexer(std::move(lexer)), current_token()
    {
    }

    ProgramPtr parse();
};

#endif