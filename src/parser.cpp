#include "parser.hpp"
#include "exceptions.hpp"
#include "operators.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <tuple>

std::map<TokenType, std::shared_ptr<BuiltInBinOp>> Parser::binary_map{
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

std::map<TokenType, std::shared_ptr<BuiltInUnaryOp>> Parser::unary_map{
    {TokenType::INCREMENT,
     std::make_shared<BuiltInUnaryOp>(BuiltInUnaryOp(60, UnaryOpEnum::INC))},
    {TokenType::DECREMENT,
     std::make_shared<BuiltInUnaryOp>(BuiltInUnaryOp(60, UnaryOpEnum::DEC))},
    {TokenType::NEG,
     std::make_shared<BuiltInUnaryOp>(BuiltInUnaryOp(60, UnaryOpEnum::NEG))},
    {TokenType::BIT_NEG, std::make_shared<BuiltInUnaryOp>(
                             BuiltInUnaryOp(60, UnaryOpEnum::BIT_NEG))},
};

std::map<TokenType, TypeEnum> Parser::type_map{
    {TokenType::TYPE_I32, TypeEnum::I32},
    {TokenType::TYPE_F64, TypeEnum::F64},
};

std::map<TokenType, TypeEnum> Parser::type_value_map{
    {TokenType::INT, TypeEnum::I32},
    {TokenType::FLOAT, TypeEnum::F32},
    {TokenType::DOUBLE, TypeEnum::F64},
};

Token Parser::get_new_token()
{
    return this->current_token = this->lexer->get_token();
}

void Parser::assert_current_and_eat(TokenType type,
                                    const std::wstring &error_msg)
{
    if (this->current_token != type)
        this->report_error(error_msg);
    this->get_new_token();
}

void Parser::assert_next_token(TokenType type, const std::wstring &error_msg)
{
    auto new_token = this->get_new_token();
    if (new_token != type)
        this->report_error(error_msg);
}

// I32 = TYPE_I32
std::unique_ptr<I32Expr> Parser::parse_i32()
{
    auto result = std::make_unique<I32Expr>(
        std::get<int32_t>(this->current_token.value));
    this->get_new_token();
    return result;
}

std::unique_ptr<F32Expr> Parser::parse_f32()
{
    auto result =
        std::make_unique<F32Expr>(std::get<double>(this->current_token.value));
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
                            L"not a colon in a parameter expression");
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
            this->get_new_token();
    }
    return params;
}

// Type = FunctionType | SimpleType | NeverType
// SimpleType = TYPE_*
TypePtr Parser::parse_type()
{
    if (this->current_token == TokenType::KW_FN)
        return this->parse_function_type();
    else if (this->type_map.contains(this->current_token.type))
    {

        auto result = std::make_unique<SimpleType>(
            this->type_map[this->current_token.type]);
        this->get_new_token();
        return result;
    }
    else
        return this->report_error(L"invalid type syntax");
}

// Types = [Type, {",", Type}]
std::vector<TypePtr> Parser::parse_types()
{
    std ::vector<TypePtr> types;
    for (; this->current_token != TokenType::R_PAREN; this->get_new_token())
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
        L"no left parenthesis in the function type definition");
    this->get_new_token();

    auto types = this->parse_types();

    this->assert_current_and_eat(
        TokenType::R_PAREN,
        L"no right parenthesis in the function type definition");

    TypePtr return_type = this->parse_return_type();
    return std::make_unique<FunctionType>(types, return_type);
}

// ExternStmt = KW_EXTERN, IDENTIFIER,
// L_PAREN, [Parameter, {COMMA, Parameter}], R_PAREN, [LAMBDA_ARROW, Type],
// SEMICOLON;
std::unique_ptr<ExternStmt> Parser::parse_extern()
{
    this->assert_next_token(TokenType::IDENTIFIER,
                            L"not an identifier in an extern declaration");
    auto name = std::get<std::wstring>(this->current_token.value);
    this->assert_next_token(
        TokenType::L_PAREN,
        L"not a left parenthesis in an extern declaration");

    auto params = this->parse_params();

    this->assert_current_and_eat(
        TokenType::R_PAREN,
        L"not a right parenthesis in an extern declaration");

    auto return_type = this->parse_return_type();

    this->assert_current_and_eat(TokenType::SEMICOLON,
                                 L"not a semicolon in an extern declaration");

    return std::make_unique<ExternStmt>(name, params, return_type);
}

// Function = KW_FN, IDENTIFIER, L_PAREN, Params, R_PAREN,
// [LAMBDA_ARROW, Type], Block, SEMICOLON;
std::unique_ptr<FuncDefStmt> Parser::parse_function()
{
    this->assert_next_token(TokenType::IDENTIFIER,
                            L"not a function identifier");
    auto name = std::get<std::wstring>(this->current_token.value);
    this->assert_next_token(
        TokenType::L_PAREN,
        L"left parenthesis missing in a function definition");
    auto params = this->parse_params();
    this->assert_current_and_eat(TokenType::R_PAREN,
                                 L"right parenthesis missing "
                                 "in a function "
                                 "definition");
    auto return_type = this->parse_return_type();
    auto block = this->parse_block();
    return std::make_unique<FuncDefStmt>(name, params, return_type, block);
}

std::optional<TypePtr> Parser::parse_var_type()
{
    std::optional<TypePtr> type = {};
    if (this->get_new_token() == TokenType::COLON)
    {
        this->get_new_token();
        type = this->parse_type();
    }
    return type;
}

std::optional<ExprNodePtr> Parser::parse_var_value()
{
    std::optional<ExprNodePtr> value = {};
    if (this->current_token == TokenType::ASSIGN)
    {
        this->get_new_token();
        value = this->parse_expression();
    }
    return value;
}

// VarDeclStmt = KW_LET, IDENTIFIER, [COLON, Type], [ASSIGN, Expression];
std::unique_ptr<VarDeclStmt> Parser::parse_variable_declaration()
{
    this->assert_next_token(
        TokenType::IDENTIFIER,
        L"expected an identifier in a variable declaration");

    auto name = std::get<std::wstring>(this->current_token.value);
    auto type = this->parse_var_type();
    auto initial_value = this->parse_var_value();

    this->assert_current_and_eat(
        TokenType::SEMICOLON, L"no semicolon found in a variable declaration");

    return std::make_unique<VarDeclStmt>(name, type, initial_value);
}

std::unique_ptr<ReturnStmt> Parser::parse_return_statement()
{
    this->get_new_token();
    auto expr = this->parse_expression();
    this->assert_current_and_eat(TokenType::SEMICOLON,
                                 L"no semicolon found in a return statement");
    return std::make_unique<ReturnStmt>(expr);
}

std::unique_ptr<Statement> Parser::parse_block_statement()
{
    if (this->current_token == TokenType::KW_LET)
        return this->parse_variable_declaration();
    else if (this->current_token == TokenType::KW_RETURN)
        return this->parse_return_statement();
    else if (this->current_token == TokenType::KW_EXTERN)
        return this->parse_extern();
    else if (this->current_token == TokenType::KW_FN)
        return this->parse_function();
    else if (this->current_token == TokenType::L_BRACKET)
        return this->parse_block();
    else
        return this->report_error(L"invalid token found in a block");
}

// Block = {Statement}
std::unique_ptr<Block> Parser::parse_block()
{
    std::vector<std::unique_ptr<Statement>> statements;
    this->get_new_token();
    for (; this->current_token != TokenType::R_BRACKET;)
        statements.push_back(this->parse_block_statement());
    this->get_new_token();
    return std::make_unique<Block>(statements);
}

std::unique_ptr<ExprNode> Parser::parse_paren_expression()
{
    this->get_new_token();
    auto expr = this->parse_expression();
    this->assert_current_and_eat(
        TokenType::R_PAREN, L"expected a right parenthesis in the expression");
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
    case TokenType::FLOAT:
        return this->parse_f32();
    case TokenType::DOUBLE:
        return this->parse_f64();
    default:
        return this->report_error(L"type token expected");
    }
}

std::vector<std::unique_ptr<ExprNode>> Parser::parse_call_args()
{

    std::vector<std::unique_ptr<ExprNode>> args;
    if (this->get_new_token() != TokenType::R_PAREN)
    {
        for (;;)
        {
            args.push_back(this->parse_expression());
            if (this->current_token == TokenType::R_PAREN)
            {
                this->get_new_token();
                break;
            }
            this->assert_current_and_eat(
                TokenType::COMMA, L"expected ')' or ',' in argument list");
        }
    }
    return args;
}

std::vector<std::optional<ExprNodePtr>> convert_to_opt_args(
    std::vector<ExprNodePtr> &&args)
{
    auto result = std::vector<std::optional<ExprNodePtr>>(args.size());
    std::transform(args.begin(), args.end(), result.begin(),
                   [](ExprNodePtr &node) {
                       return std::make_optional<ExprNodePtr>(std::move(node));
                   });
    return result;
}

void convert_if_not_lambda(
    std::vector<std::unique_ptr<ExprNode>> &args,
    std::vector<std::optional<std::unique_ptr<ExprNode>>> &lambda_args,
    bool &is_lambda)
{
    if (!is_lambda)
    {
        lambda_args = convert_to_opt_args(std::move(args));
        is_lambda = true;
    }
}

std::unique_ptr<LambdaCallExpr> Parser::return_ellipsis_lambda(
    const std::wstring &name, std::vector<std::unique_ptr<ExprNode>> &args,
    std::vector<std::optional<std::unique_ptr<ExprNode>>> &lambda_args,
    bool &is_lambda)
{

    convert_if_not_lambda(args, lambda_args, is_lambda);
    this->assert_next_token(TokenType::R_PAREN,
                            L"expected ')' after ellipsis in argument list");
    this->get_new_token();
    return std::make_unique<LambdaCallExpr>(name, lambda_args, true);
}

bool Parser::eat_comma_or_rparen()
{
    if (this->current_token == TokenType::R_PAREN)
    {
        this->get_new_token();
        return true;
    }
    this->assert_current_and_eat(TokenType::COMMA,
                                 L"expected ')' or ',' in argument list");
    return false;
}

ExprNodePtr return_call_or_lambda(
    const std::wstring &name, std::vector<ExprNodePtr> &args,
    std::vector<std::optional<ExprNodePtr>> &lambda_args,
    const bool &is_lambda)
{
    if (is_lambda)
        return std::make_unique<LambdaCallExpr>(name, lambda_args, false);
    else
        return std::make_unique<CallExpr>(name, args);
}

void Parser::push_expr(std::vector<ExprNodePtr> &args,
                       std::vector<std::optional<ExprNodePtr>> &lambda_args,
                       const bool &is_lambda)
{
    if (is_lambda)
        lambda_args.push_back(this->parse_expression());
    else
        args.push_back(this->parse_expression());
}

void Parser::handle_placeholder(
    std::vector<ExprNodePtr> &args,
    std::vector<std::optional<ExprNodePtr>> &lambda_args, bool &is_lambda)
{

    convert_if_not_lambda(args, lambda_args, is_lambda);
    lambda_args.push_back(std::nullopt);
    this->get_new_token();
}

std::unique_ptr<ExprNode> Parser::parse_call_or_lambda(
    const std::wstring &name)
{
    std::vector<ExprNodePtr> args;
    std::vector<std::optional<ExprNodePtr>> lambda_args;
    auto is_lambda = false;

    if (this->get_new_token() != TokenType::R_PAREN)
    {
        for (;;)
        {
            if (this->current_token == TokenType::PLACEHOLDER)
            {
                this->handle_placeholder(args, lambda_args, is_lambda);
            }
            else if (this->current_token == TokenType::ELLIPSIS)
            {
                return this->return_ellipsis_lambda(name, args, lambda_args,
                                                    is_lambda);
            }
            else
            {
                this->push_expr(args, lambda_args, is_lambda);
            }
            if (this->eat_comma_or_rparen())
                break;
        }
    }
    return return_call_or_lambda(name, args, lambda_args, is_lambda);
}

std::unique_ptr<ExprNode> Parser::parse_identifier_expression()
{
    auto name = std::get<std::wstring>(this->current_token.value);
    this->get_new_token();

    if (this->current_token != TokenType::L_PAREN)
        return std::make_unique<VariableExpr>(name);

    return this->parse_call_or_lambda(name);
}

std::unique_ptr<ExprNode> Parser::parse_lhs()
{
    if (this->current_token == TokenType::IDENTIFIER)
        return this->parse_identifier_expression();
    else if (this->current_token == TokenType::L_PAREN)
        return this->parse_paren_expression();
    else if (this->type_value_map.contains(this->current_token.type))
        return this->parse_const_expression();
    else
        return this->report_error(
            L"unknown token when expecting an expression");
}

void Parser::check_next_op_and_parse(std::unique_ptr<ExprNode> &lhs,
                                     std::unique_ptr<ExprNode> &rhs,
                                     const std::shared_ptr<BuiltInBinOp> &op)
{
    if (this->binary_map.contains(this->current_token.type))
    {
        auto new_op = this->binary_map.at(this->current_token.type);
        if (op->precedence < new_op->precedence)
            rhs = this->parse_op_and_rhs(op->precedence + 1, rhs);
    }
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
        this->check_next_op_and_parse(lhs, rhs, op);
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

std::nullptr_t Parser::report_error(const std::wstring &msg)
{
    auto error_msg = build_wstring(
        L"[ERROR] Parser error at [", this->lexer->get_position().line, ",",
        this->lexer->get_position().column, "]: ", msg, ".");
    throw ParserException(error_msg);
    return nullptr;
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
        switch (this->current_token.type)
        {
        case TokenType::KW_FN:
            functions.push_back(this->parse_function());
            break;
        case TokenType::KW_EXTERN:
            externs.push_back(this->parse_extern());
            break;
        case TokenType::KW_LET:
            globals.push_back(this->parse_variable_declaration());
            break;
        default:
            this->report_error(L"unrecognized token");
        }
    }
    return std::make_unique<Program>(globals, functions, externs);
}
