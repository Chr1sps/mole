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
    std::map<TokenType, std::shared_ptr<BuiltInBinOp>> binary_map = {
        {TokenType::PLUS,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(45, BinOpEnum::ADD))},
        {TokenType::MINUS,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(45, BinOpEnum::SUB))},
        {TokenType::STAR,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(50, BinOpEnum::MUL))},
        {TokenType::SLASH,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(50, BinOpEnum::DIV))},
        {TokenType::PERCENT,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(50, BinOpEnum::MOD))},
        {TokenType::EXP,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(55, BinOpEnum::EXP))},
        {TokenType::EQUAL,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(20, BinOpEnum::EQ))},
        {TokenType::NOT_EQUAL,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(20, BinOpEnum::NEQ))},
        {TokenType::GREATER,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(20, BinOpEnum::GT))},
        {TokenType::GREATER_EQUAL,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(20, BinOpEnum::GE))},
        {TokenType::LESS,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(20, BinOpEnum::LT))},
        {TokenType::LESS_EQUAL,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(20, BinOpEnum::LE))},
        {TokenType::AND,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(15, BinOpEnum::AND))},
        {TokenType::OR,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(10, BinOpEnum::OR))},
        {TokenType::AMPERSAND,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(35, BinOpEnum::BIT_AND))},
        {TokenType::BIT_OR,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(25, BinOpEnum::BIT_OR))},
        {TokenType::BIT_XOR,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(30, BinOpEnum::BIT_XOR))},
        {TokenType::SHIFT_LEFT,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(40, BinOpEnum::SHL))},
        {TokenType::SHIFT_RIGHT,
         std::make_shared<BuiltInBinOp>(BuiltInBinOp(40, BinOpEnum::SHR))},
    };
    std::map<TokenType, std::shared_ptr<BuiltInUnaryOp>> unary_map = {
        {TokenType::INCREMENT, std::make_shared<BuiltInUnaryOp>(
                                   BuiltInUnaryOp(60, UnaryOpEnum::INC))},
        {TokenType::DECREMENT, std::make_shared<BuiltInUnaryOp>(
                                   BuiltInUnaryOp(60, UnaryOpEnum::DEC))},
        {TokenType::NEG, std::make_shared<BuiltInUnaryOp>(
                             BuiltInUnaryOp(60, UnaryOpEnum::NEG))},
        {TokenType::BIT_NEG, std::make_shared<BuiltInUnaryOp>(
                                 BuiltInUnaryOp(60, UnaryOpEnum::BIT_NEG))},
    };
    std::map<TokenType, TypeEnum> type_map = {
        {TokenType::TYPE_I32, TypeEnum::I32},
        {TokenType::TYPE_F64, TypeEnum::F64},
    };
    std::map<TokenType, TypeEnum> type_value_map = {
        {TokenType::INT, TypeEnum::I32},
        {TokenType::DOUBLE, TypeEnum::F64},
    };

    Token get_new_token();
    // Token peek_token();

    void assert_current_token(TokenType type, const char *error_msg);
    void assert_next_token(TokenType type, const char *error_msg);

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
    std::unique_ptr<ExternStmt> parse_extern();
    std::unique_ptr<VarDeclStmt> parse_variable_declaration();
    std::unique_ptr<ReturnStmt> parse_return_statement();
    std::unique_ptr<Statement> parse_block_statement();
    std::unique_ptr<Block> parse_block();

    // expressions
    std::unique_ptr<I32Expr> parse_i32();
    std::unique_ptr<F64Expr> parse_f64();
    std::unique_ptr<ExprNode> parse_paren_expression();
    std::unique_ptr<ExprNode> parse_unary_expression();
    std::unique_ptr<ExprNode> parse_identifier_expression();
    std::unique_ptr<ExprNode> parse_const_expression();
    std::unique_ptr<ExprNode> parse_lhs();
    std::unique_ptr<ExprNode> parse_op_and_rhs(const unsigned &precedence,
                                               std::unique_ptr<ExprNode> &lhs);
    std::unique_ptr<ExprNode> parse_expression();

  public:
    Parser(LexerPtr &lexer) : lexer(std::move(lexer))
    {
    }

    ProgramPtr parse();
};

class ParserException : public std::exception
{
    const char *what_str;

  public:
    ParserException() = default;

    ParserException(const char *what_str) : what_str(what_str)
    {
    }

    const char *what() const noexcept override
    {
        return this->what_str;
    }
};
#endif