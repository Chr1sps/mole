#include "parser.hpp"

std::unique_ptr<I32Expr> Parser::parse_i32()
{
    return std::make_unique<I32Expr>(this->current_token.value);
}

std::unique_ptr<F64Expr> Parser::parse_f64()
{
    return std::make_unique<F64Expr>(this->current_token.value);
}

std::unique_ptr<FuncDefStmt> Parser::parse_function()
{
}

// ExternStmt = KW_EXTERN, IDENTIFIER,
// L_PAREN, [Parameter, {COMMA, Parameter}], R_PAREN, SEMICOLON;
std::unique_ptr<ExternStmt> Parser::parse_extern()
{
    if (this->get_new_token() != TokenType::IDENTIFIER)

        throw ParserException("Not an identifier in an extern declaration!");
}

// VarDeclStmt = KW_LET, IDENTIFIER, ASSIGN
std::unique_ptr<VarDeclStmt> Parser::parse_variable_declaration()
{
}

ProgramPtr Parser::parse()
{
    std::vector<std::unique_ptr<VarDeclStmt>> globals;
    std::vector<std::unique_ptr<FuncDefStmt>> functions;
    std::vector<std::unique_ptr<ExternStmt>> externs;
    for (this->get_new_token(); this->current_token != TokenType::END;)
    {
        if (this->current_token == TokenType::KW_FN)
            functions.push_back(this->parse_function());
        else if (this->current_token == TokenType::KW_EXTERN)
            externs.push_back(this->parse_extern());
        else if (this->current_token == TokenType::KW_LET)
            globals.push_back(this->parse_variable_declaration());
        else
            throw ParserException("Unrecognised token");
    }
    return std::make_unique<Program>(globals, functions, externs);
}
