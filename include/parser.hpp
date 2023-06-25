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
    std::map<TokenType, BinaryOperator> binary_map = {
        {TokenType::PLUS, BinaryOperator::ADD},
        {TokenType::MINUS, BinaryOperator::SUB},
        {TokenType::STAR, BinaryOperator::MUL},
        {TokenType::SLASH, BinaryOperator::DIV},
        {TokenType::PERCENT, BinaryOperator::MOD},
        {TokenType::EXP, BinaryOperator::EXP},
        {TokenType::EQUAL, BinaryOperator::EQ},
        {TokenType::NOT_EQUAL, BinaryOperator::NEQ},
        {TokenType::GREATER, BinaryOperator::GT},
        {TokenType::GREATER_EQUAL, BinaryOperator::GE},
        {TokenType::LESS, BinaryOperator::LT},
        {TokenType::LESS_EQUAL, BinaryOperator::LE},
        {TokenType::AND, BinaryOperator::AND},
        {TokenType::OR, BinaryOperator::OR},
        {TokenType::AMPERSAND, BinaryOperator::BIT_AND},
        {TokenType::BIT_OR, BinaryOperator::BIT_OR},
        {TokenType::BIT_XOR, BinaryOperator::BIT_XOR},
        {TokenType::SHIFT_LEFT, BinaryOperator::SHL},
        {TokenType::SHIFT_RIGHT, BinaryOperator::SHR},
    };
    std::map<TokenType, UnaryOperator> unary_map = {
        {TokenType::INCREMENT, UnaryOperator::INC},
        {TokenType::DECREMENT, UnaryOperator::DEC},
        {TokenType::NEG, UnaryOperator::NEG},
        {TokenType::BIT_NEG, UnaryOperator::BIT_NEG},
    };

    Token get_new_token();

    std::unique_ptr<I32Expr> parse_i32();
    std::unique_ptr<F64Expr> parse_f64();

    std::unique_ptr<FuncDefStmt> parse_function();
    std::unique_ptr<ExternStmt> parse_extern();
    std::unique_ptr<VarDeclStmt> parse_variable_declaration();

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