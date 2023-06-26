#include "parser.hpp"

Token Parser::get_new_token()
{
    return this->current_token = this->lexer->get_token();
}

void Parser::assert_current_token(TokenType type, const char *error_msg)
{
    if (this->current_token != type)
        throw ParserException(error_msg);
}

void Parser::assert_next_token(TokenType type, const char *error_msg)
{
    auto new_token = this->get_new_token();
    if (new_token != type)
        throw ParserException(error_msg);
}

// I32 = TYPE_I32
std::unique_ptr<I32Expr> Parser::parse_i32()
{
    auto result = std::make_unique<I32Expr>(
        std::get<int32_t>(this->current_token.value));
    this->get_new_token();
    return result;
}

// F64 = TYPE_F64
std::unique_ptr<F64Expr> Parser::parse_f64()
{
    auto result =
        std::make_unique<F64Expr>(std::get<double>(this->current_token.value));
    this->get_new_token();
    return result;
}

// Parameter = IDENTIFIER, COLON, Type
ParamPtr Parser::parse_param()
{
    auto name = std::get<std::wstring>(this->current_token.value);
    this->assert_next_token(TokenType::COLON,
                            "Not a colon in a parameter expression!");
    this->get_new_token();
    auto param_type = this->parse_type();
    return std::make_unique<Parameter>(name, std::move(param_type));
}

// Params = [Parameter, {COMMA, Parameter}]
std::vector<ParamPtr> Parser::parse_params()
{
    std::vector<ParamPtr> params;
    for (this->get_new_token(); this->current_token == TokenType::IDENTIFIER;)
    {
        auto parameter = this->parse_param();
        params.push_back(std::move(parameter));
        if (this->current_token == TokenType::COMMA)
        {
            this->get_new_token();
        }
    }
    return params;
}

// Type = FunctionType | SimpleType | NeverType
// SimpleType = TYPE_*
TypePtr Parser::parse_type()
{
    if (this->current_token == TokenType::KW_FN)
    {
        return this->parse_function_type();
    }
    else if (this->type_map.contains(this->current_token.type))
    {

        auto result = std::make_unique<SimpleType>(
            this->type_map[this->current_token.type]);
        this->get_new_token();
        return result;
    }
    else
        throw ParserException("Invalid type syntax.");
}

// Types = [Type, {",", Type}]
std::vector<TypePtr> Parser::parse_types()
{
    std::vector<TypePtr> types;
    for (this->get_new_token();; this->get_new_token())
    {
        auto type = this->parse_type();
        types.push_back(std::move(type));
        if (this->current_token != TokenType::COMMA)
            break;
    }
    return types;
}

// ReturnType = [LAMBDA_ARROW, Type]
TypePtr Parser::parse_return_type()
{
    TypePtr return_type;
    if (this->current_token == TokenType::LAMBDA_ARROW)
    {
        this->get_new_token();
        return_type = this->parse_type();
    }
    else
    {
        return_type = std::make_unique<NeverType>();
    }
    return return_type;
}

// FunctionType = KW_FN, L_PAREN, Types, R_PAREN,
// [LAMBDA_ARROW, Type]
std::unique_ptr<FunctionType> Parser::parse_function_type()
{
    this->assert_next_token(
        TokenType::L_PAREN,
        "No left parenthesis in the function type definition.");

    auto types = this->parse_types();
    if (this->current_token != TokenType::R_PAREN)
        throw ParserException(
            "No right parenthesis in the function type definition.");

    this->get_new_token();

    TypePtr return_type = this->parse_return_type();
    return std::make_unique<FunctionType>(types, return_type);
}

// ExternStmt = KW_EXTERN, IDENTIFIER,
// L_PAREN, [Parameter, {COMMA, Parameter}], R_PAREN, [LAMBDA_ARROW, Type],
// SEMICOLON;
std::unique_ptr<ExternStmt> Parser::parse_extern()
{
    this->assert_next_token(TokenType::IDENTIFIER,
                            "Not an identifier in an extern declaration.");
    auto name = std::get<std::wstring>(this->current_token.value);
    this->assert_next_token(
        TokenType::L_PAREN,
        "Not a left parenthesis in an extern declaration.");

    auto params = this->parse_params();

    if (this->current_token != TokenType::R_PAREN)
    {
        throw ParserException(
            "Not a right parenthesis in an extern declaration.");
    }
    this->get_new_token();

    auto return_type = this->parse_return_type();

    this->assert_current_token(TokenType::SEMICOLON,
                               "Not a semicolon in an extern declaration.");

    this->get_new_token();
    return std::make_unique<ExternStmt>(name, params, return_type);
}

// Function = KW_FN, IDENTIFIER, L_PAREN, Params, R_PAREN,
// [LAMBDA_ARROW, Type], Block, SEMICOLON;
std::unique_ptr<FuncDefStmt> Parser::parse_function()
{
    this->assert_next_token(TokenType::IDENTIFIER,
                            "Not a function identifier.");
    auto name = std::get<std::wstring>(this->current_token.value);
    this->assert_next_token(
        TokenType::L_PAREN,
        "Left parenthesis missing in a function definition.");
    auto params = this->parse_params();
    if (this->current_token != TokenType::R_PAREN)
        throw ParserException(
            "Right parenthesis missing in a function definition.");
    this->get_new_token();
    auto return_type = this->parse_return_type();
    auto block = this->parse_block();
    return std::make_unique<FuncDefStmt>(name, params, return_type, block);
}

// VarDeclStmt = KW_LET, IDENTIFIER, [COLON, Type], [ASSIGN, Expression];
std::unique_ptr<VarDeclStmt> Parser::parse_variable_declaration()
{
    this->assert_next_token(
        TokenType::IDENTIFIER,
        "Expected an identifier in a variable declaration.");
    auto name = std::get<std::wstring>(this->current_token.value);

    std::optional<TypePtr> type = {};
    if (this->get_new_token() == TokenType::COLON)
    {
        this->get_new_token();
        type = this->parse_type();
    }

    std::optional<ExprNodePtr> initial_value = {};
    if (this->current_token == TokenType::ASSIGN)
    {
        this->get_new_token();
        initial_value = this->parse_expression();
    }

    this->assert_current_token(
        TokenType::SEMICOLON, "No semicolon found in a variable declaration.");
    this->get_new_token();

    return std::make_unique<VarDeclStmt>(name, type, initial_value);
}

std::unique_ptr<ReturnStmt> Parser::parse_return_statement()
{
    this->get_new_token();
    auto expr = this->parse_expression();
    if (this->current_token != TokenType::SEMICOLON)
        throw ParserException("No semicolon found in a return statement.");
    this->get_new_token();
    return std::make_unique<ReturnStmt>(expr);
}

// Block = {Statement}
std::unique_ptr<Block> Parser::parse_block()
{
    std::vector<std::unique_ptr<Statement>> statements;
    this->get_new_token();
    for (;;)
    {
        if (this->current_token == TokenType::KW_LET)
            statements.push_back(this->parse_variable_declaration());
        else if (this->current_token == TokenType::KW_RETURN)
            statements.push_back(this->parse_return_statement());
        else if (this->current_token == TokenType::KW_EXTERN)
            statements.push_back(this->parse_extern());
        else if (this->current_token == TokenType::KW_FN)
            statements.push_back(this->parse_function());
        else if (this->current_token == TokenType::R_BRACKET)
            break;
        else
            throw ParserException("Invalid token found in a block.");
    }
    this->get_new_token();
    return std::make_unique<Block>(statements);
}

std::unique_ptr<ExprNode> Parser::parse_paren_expression()
{
    this->get_new_token();
    auto expr = this->parse_expression();
    if (this->current_token != TokenType::R_PAREN)
        throw ParserException(
            "Expected a right parenthesis in the expression.");
    this->get_new_token();
    return expr;
}

std::unique_ptr<ExprNode> Parser::parse_unary_expression()
{
    auto op = this->unary_map.at(this->current_token.type);
    this->get_new_token();
    auto expr = this->parse_expression();
    return std::make_unique<UnaryExpr>(expr, op);
}

std::unique_ptr<ExprNode> Parser::parse_const_expression()
{
    switch (this->current_token.type)
    {
    case TokenType::INT:
        return this->parse_i32();
    case TokenType::DOUBLE:
        return this->parse_f64();
    default:
        throw ParserException("Type token expected.");
    }
}

std::unique_ptr<ExprNode> Parser::parse_variable_expression()
{
    auto value = std::get<std::wstring>(this->current_token.value);
    this->get_new_token();
    return std::make_unique<VariableExpr>(value);
}

std::unique_ptr<ExprNode> Parser::parse_lhs()
{
    if (this->current_token == TokenType::IDENTIFIER)
        return this->parse_variable_expression();
    else if (this->current_token == TokenType::L_PAREN)
        return this->parse_paren_expression();
    else if (this->type_value_map.contains(this->current_token.type))
    {
        return this->parse_const_expression();
    }
    else
        throw ParserException("Unknown token when expecting an expression.");
}

std::unique_ptr<ExprNode> Parser::parse_op_and_rhs(
    const unsigned &precedence, std::unique_ptr<ExprNode> &lhs)
{
    for (;;)
    {
        if (!this->binary_map.contains(this->current_token.type))
            return std::move(lhs);
        auto op = this->binary_map.at(this->current_token.type);
        if (op->precedence < precedence)
            return std::move(lhs);
        this->get_new_token();
        auto rhs = parse_lhs();
        auto new_op = this->binary_map.at(this->current_token.type);
        if (op->precedence < new_op->precedence)
        {
            rhs = this->parse_op_and_rhs(op->precedence + 1, rhs);
        }
        lhs = std::make_unique<BinaryExpr>(lhs, rhs, op);
    }
}

std::unique_ptr<ExprNode> Parser::parse_expression()
{
    auto lhs = this->parse_lhs();
    if (this->binary_map.contains(this->current_token.type))
        return this->parse_op_and_rhs(0, lhs);
    return lhs;
}

// Program = VarDeclStmt | FuncDefStmt | ExternStmt,
// {VarDeclStmt | FuncDefStmt | ExternStmt}
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
