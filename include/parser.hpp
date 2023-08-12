#ifndef __PARSER_HPP__
#define __PARSER_HPP__
#include "ast.hpp"
#include "lexer.hpp"
#include "operators.hpp"
#include "token.hpp"
#include <map>

class Parser
{
    LexerPtr lexer;
    Token current_token;
    static std::map<TokenType, std::shared_ptr<BuiltInBinOp>> binary_map;
    static std::map<TokenType, std::shared_ptr<BuiltInUnaryOp>> unary_map;
    static std::map<TokenType, TypeEnum> type_map;
    static std::map<TokenType, TypeEnum> type_value_map;
    static std::map<TokenType, AssignType> assign_map;

    Token get_new_token();

    void assert_current_and_eat(TokenType type, const std::wstring &error_msg);
    void assert_next_token(TokenType type, const std::wstring &error_msg);

    // type names

    TypePtr parse_type();
    std::vector<TypePtr> parse_types();
    TypePtr parse_return_type();
    std::unique_ptr<FunctionType> parse_function_type();

    // function parameters

    ParamPtr parse_param();
    std::vector<ParamPtr> parse_params();

    // statements

    std::unique_ptr<FuncDefStmt> parse_function();
    std::optional<TypePtr> parse_var_type();
    std::optional<ExprNodePtr> parse_var_value();
    std::unique_ptr<ExternStmt> parse_extern();
    std::unique_ptr<VarDeclStmt> parse_variable_declaration();
    std::unique_ptr<ReturnStmt> parse_return_statement();
    std::unique_ptr<AssignStmt> parse_assign_statement();
    std::unique_ptr<Statement> parse_block_statement();
    std::unique_ptr<Block> parse_block();

    // helper methods

    void check_next_op_and_parse(std::unique_ptr<ExprNode> &lhs,
                                 std::unique_ptr<ExprNode> &rhs,
                                 const std::shared_ptr<BuiltInBinOp> &op);
    void push_expr(std::vector<ExprNodePtr> &args,
                   std::vector<std::optional<ExprNodePtr>> &lambda_args,
                   const bool &is_lambda);

    void handle_placeholder(
        std::vector<ExprNodePtr> &args,
        std::vector<std::optional<ExprNodePtr>> &lambda_args, bool &is_lambda);
    bool eat_comma_or_rparen();
    std::unique_ptr<LambdaCallExpr> return_ellipsis_lambda(
        const std::wstring &name, std::vector<std::unique_ptr<ExprNode>> &args,
        std::vector<std::optional<std::unique_ptr<ExprNode>>> &lambda_args,
        bool &is_lambda);
    std::unique_ptr<LambdaCallExpr> handle_call_and_lambda_args(
        const std::wstring &name, std::vector<ExprNodePtr> &args,
        std::vector<std::optional<ExprNodePtr>> &lambda_args, bool &is_lambda);
    // expressions

    std::unique_ptr<I32Expr> parse_i32();
    std::unique_ptr<F64Expr> parse_f64();
    std::unique_ptr<ExprNode> parse_paren_expression();
    std::unique_ptr<ExprNode> parse_unary_expression();
    std::unique_ptr<ExprNode> parse_call_or_lambda(const std::wstring &name);
    std::unique_ptr<ExprNode> parse_identifier_expression();
    std::unique_ptr<ExprNode> parse_const_expression();
    std::vector<std::unique_ptr<ExprNode>> parse_call_args();
    std::unique_ptr<ExprNode> parse_lhs();
    std::unique_ptr<ExprNode> parse_op_and_rhs(const unsigned &precedence,
                                               std::unique_ptr<ExprNode> &lhs);
    std::unique_ptr<ExprNode> parse_expression();

    std::nullptr_t report_error(const std::wstring &error_msg);

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