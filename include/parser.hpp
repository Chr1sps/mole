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

    Token get_new_token();
    // Token peek_token();

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
    std::unique_ptr<Statement> parse_block_statement();
    std::unique_ptr<Block> parse_block();

    // expressions
    std::unique_ptr<I32Expr> parse_i32();
    std::unique_ptr<F32Expr> parse_f32();
    std::unique_ptr<F64Expr> parse_f64();
    std::unique_ptr<ExprNode> parse_paren_expression();
    std::unique_ptr<ExprNode> parse_unary_expression();
    std::unique_ptr<ExprNode> parse_identifier_expression();
    std::unique_ptr<ExprNode> parse_const_expression();
    std::vector<std::unique_ptr<ExprNode>> parse_call_args();
    std::unique_ptr<ExprNode> parse_lhs();
    void check_next_op_and_parse(std::unique_ptr<ExprNode> &lhs,
                                 std::unique_ptr<ExprNode> &rhs,
                                 const std::shared_ptr<BuiltInBinOp> &op);
    std::unique_ptr<ExprNode> parse_op_and_rhs(const unsigned &precedence,
                                               std::unique_ptr<ExprNode> &lhs);
    std::unique_ptr<ExprNode> parse_expression();

    std::nullptr_t report_error(const std::wstring &error_msg);

  public:
    Parser(LexerPtr &lexer) : lexer(std::move(lexer))
    {
    }

    Parser(LexerPtr &&lexer) : lexer(std::move(lexer))
    {
    }

    ProgramPtr parse();
};

#endif