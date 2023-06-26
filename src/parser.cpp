#include "parser.hpp"

Token Parser::get_new_token()
{
    return this->current_token = this->lexer->get_token();
}

void Parser::assert_next_token(TokenType type, const char *error_msg)
{
    if (this->get_new_token() != type)
        throw ParserException(error_msg);
}

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

ParamPtr Parser::parse_param()
{
    this->assert_next_token(TokenType::COLON,
                            "Not a colon in a parameter expression!");
    auto param_type = this->parse_type();
}

TypePtr Parser::parse_type()
{
    if (this->current_token == TokenType::KW_FN)
    {
        return this->parse_function_type();
    }
    else if (this->type_map.contains(this->current_token.type))
    {
        return std::make_unique<Type>(
            this->type_map[this->current_token.type]);
    }
    else
        throw ParserException("Invalid type syntax.");
}

std::unique_ptr<FunctionType> Parser::parse_function_type()
{
    this->assert_next_token(
        TokenType::L_PAREN,
        "No left parenthesis in the function type definition.");

    std::vector<TypePtr> types;
    for (this->get_new_token();;)
    {
        auto type = this->parse_type();
        types.push_back(type);
        if (this->current_token != TokenType::COMMA)
            break;
    }
    this->assert_next_token(
        TokenType::R_PAREN,
        "No right parenthesis in the function type definition.");

    TypePtr return_type;
    if (this->get_new_token() == TokenType::LAMBDA_ARROW)
    {
        return_type = this->parse_type();
    }
    else
        return_type = std::make_unique<NeverType>();
    return std::make_unique<FunctionType>(types, return_type);
}

// ExternStmt = KW_EXTERN, IDENTIFIER,
// L_PAREN, [Parameter, {COMMA, Parameter}], R_PAREN, SEMICOLON;
std::unique_ptr<ExternStmt> Parser::parse_extern()
{
    std::vector<ParamPtr> params;
    this->assert_next_token(TokenType::IDENTIFIER,
                            "Not an identifier in an extern declaration.");
    this->assert_next_token(
        TokenType::L_PAREN,
        "Not a left parenthesis in an extern declaration.");
    for (this->get_new_token(); this->current_token == TokenType::IDENTIFIER;)
    {
        auto parameter = this->parse_param();
        if (this->current_token == TokenType::COMMA)
        {
        }
    }

    this->assert_next_token(
        TokenType::R_PAREN,
        "Not a right parenthesis in an extern declaration.");

    this->assert_next_token(TokenType::SEMICOLON,
                            "Not a semicolon in an extern declaration.");
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
