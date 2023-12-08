#include "parser.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <optional>
#include <tuple>

namespace
{

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

} // namespace

#define BUILT_IN_BINOP(token_type, precedence, bin_op_type)                   \
    {                                                                         \
        TokenType::token_type, std::make_shared<BuiltInBinOp>(                \
                                   precedence, BinOpEnum::bin_op_type)        \
    }
std::map<TokenType, std::shared_ptr<BuiltInBinOp>> Parser::binary_map{
    BUILT_IN_BINOP(EXP, 55, EXP),           // `^^` - 55
    BUILT_IN_BINOP(STAR, 50, MUL),          // `*`, `/`, `%` - 50
    BUILT_IN_BINOP(SLASH, 50, DIV),         // |
    BUILT_IN_BINOP(PERCENT, 50, MOD),       // |
    BUILT_IN_BINOP(PLUS, 45, ADD),          // `+`, `-` - 45
    BUILT_IN_BINOP(MINUS, 45, SUB),         // |
    BUILT_IN_BINOP(SHIFT_LEFT, 40, SHL),    // `<<`, `>>` - 40
    BUILT_IN_BINOP(SHIFT_RIGHT, 40, SHR),   // |
    BUILT_IN_BINOP(AMPERSAND, 35, BIT_AND), // `&` - 35
    BUILT_IN_BINOP(BIT_XOR, 30, BIT_XOR),   // `^` - 30
    BUILT_IN_BINOP(BIT_OR, 25, BIT_OR),     // `|` - 25
    BUILT_IN_BINOP(EQUAL, 20, EQ),          // `==`,`!=`,`>`,`>=`,`<`,`<=` - 20
    BUILT_IN_BINOP(NOT_EQUAL, 20, NEQ),     // |
    BUILT_IN_BINOP(GREATER, 20, GT),        // |
    BUILT_IN_BINOP(GREATER_EQUAL, 20, GE),  // |
    BUILT_IN_BINOP(LESS, 20, LT),           // |
    BUILT_IN_BINOP(LESS_EQUAL, 20, LE),     // |
    BUILT_IN_BINOP(AND, 15, AND),           // `&&` - 15
    BUILT_IN_BINOP(OR, 10, OR),             // `||` - 10
};
#undef BUILT_IN_BINOP

#define BUILT_IN_UNOP(token_type, un_op_type)                                 \
    {                                                                         \
        TokenType::token_type,                                                \
            std::make_shared<BuiltInUnaryOp>(UnaryOpEnum::un_op_type)         \
    }
std::map<TokenType, std::shared_ptr<BuiltInUnaryOp>> Parser::unary_map{
    BUILT_IN_UNOP(INCREMENT, INC),
    BUILT_IN_UNOP(DECREMENT, DEC),
    BUILT_IN_UNOP(NEG, NEG),
    BUILT_IN_UNOP(BIT_NEG, BIT_NEG),
};
#undef BUILT_IN_UNOP

#define TYPE(token_type, type)                                                \
    {                                                                         \
        TokenType::token_type, TypeEnum::type                                 \
    }
std::map<TokenType, TypeEnum> Parser::type_map{
    TYPE(TYPE_U8, U8),   TYPE(TYPE_U16, U16),   TYPE(TYPE_U32, U32),
    TYPE(TYPE_U64, U64), TYPE(TYPE_I8, I8),     TYPE(TYPE_I16, I16),
    TYPE(TYPE_I32, I32), TYPE(TYPE_I64, I64),   TYPE(TYPE_F32, F32),
    TYPE(TYPE_F64, F64), TYPE(TYPE_BOOL, BOOL),
};
#undef TYPE

#define TYPE_VALUE(token_type, type)                                          \
    {                                                                         \
        TokenType::token_type, TypeEnum::type                                 \
    }
std::map<TokenType, TypeEnum> Parser::type_value_map{
    TYPE_VALUE(INT, I32),
    TYPE_VALUE(DOUBLE, F64),
};
#undef TYPE_VALUE

#define ASSIGN_TYPE(token_type, assign_type)                                  \
    {                                                                         \
        TokenType::token_type, AssignType::assign_type                        \
    }
std::map<TokenType, AssignType> Parser::assign_map{
    ASSIGN_TYPE(ASSIGN, NORMAL),
    ASSIGN_TYPE(ASSIGN_PLUS, PLUS),
    ASSIGN_TYPE(ASSIGN_MINUS, MINUS),
    ASSIGN_TYPE(ASSIGN_STAR, MUL),
    ASSIGN_TYPE(ASSIGN_SLASH, DIV),
    ASSIGN_TYPE(ASSIGN_PERCENT, MOD),
    ASSIGN_TYPE(ASSIGN_EXP, EXP),
    ASSIGN_TYPE(ASSIGN_BIT_NEG, BIT_NEG),
    ASSIGN_TYPE(ASSIGN_AMPERSAND, BIT_AND),
    ASSIGN_TYPE(ASSIGN_BIT_OR, BIT_OR),
    ASSIGN_TYPE(ASSIGN_BIT_XOR, BIT_XOR),
    ASSIGN_TYPE(ASSIGN_SHIFT_LEFT, SHL),
    ASSIGN_TYPE(ASSIGN_SHIFT_RIGHT, SHR),
};
#undef ASSIGN_TYPE

std::optional<Token> Parser::get_new_token()
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

void Parser::push_pos()
{
    this->position_stack.push_back(this->current_token.position);
}

Position Parser::pop_pos()
{
    auto result = this->position_stack.back();
    this->position_stack.pop_back();
    return result;
}

Position Parser::read_pos()
{
    return this->position_stack.back();
}

// I32 = INT
std::unique_ptr<I32Expr> Parser::parse_i32()
{
    auto result = std::make_unique<I32Expr>(
        std::get<unsigned long long>(this->current_token.value),
        this->current_token.position);
    this->get_new_token();
    return result;
}

// F64 = DOUBLE
std::unique_ptr<F64Expr> Parser::parse_f64()
{
    auto result =
        std::make_unique<F64Expr>(std::get<double>(this->current_token.value),
                                  this->current_token.position);
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
    else if (this->current_token.type == TokenType::NEG)
    {
        this->get_new_token();
        return std::make_unique<NeverType>();
    }
    else
        return this->report_error(L"invalid type syntax");
}

// Types = [Type, {",", Type}]
std::vector<TypePtr> Parser::parse_types()
{
    std::vector<TypePtr> types;
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
    auto is_const = false;
    this->get_new_token();
    if (this->current_token == TokenType::KW_CONST)
    {
        is_const = true;
        this->get_new_token();
    }
    this->assert_current_and_eat(
        TokenType::L_PAREN,
        L"no left parenthesis in the function type definition");

    auto types = this->parse_types();

    this->assert_current_and_eat(
        TokenType::R_PAREN,
        L"no right parenthesis in the function type definition");

    TypePtr return_type = this->parse_return_type();
    return std::make_unique<FunctionType>(types, return_type, is_const);
}

// ExternStmt = KW_EXTERN, IDENTIFIER,
// L_PAREN, [Parameter, {COMMA, Parameter}], R_PAREN, [LAMBDA_ARROW, Type],
// SEMICOLON;
std::unique_ptr<ExternStmt> Parser::parse_extern()
{
    this->push_pos();
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

    return std::make_unique<ExternStmt>(name, params, return_type,
                                        this->pop_pos());
}

// Function = KW_FN, IDENTIFIER, L_PAREN, Params, R_PAREN,
// [LAMBDA_ARROW, Type], Block;
std::unique_ptr<FuncDefStmt> Parser::parse_function()
{
    if (this->current_token != TokenType::KW_FN)
        return nullptr;
    this->push_pos();
    this->get_new_token();
    auto is_const = false;
    if (this->current_token == TokenType::KW_CONST)
    {
        is_const = true;
        this->get_new_token();
    }
    if (this->current_token != TokenType::IDENTIFIER)
        this->report_error(L"not a function identifier");
    auto name = std::get<std::wstring>((*this->current_token).value);
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
    return std::make_unique<FuncDefStmt>(name, params, return_type, block,
                                         is_const, this->pop_pos());
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
    this->push_pos();
    this->get_new_token();
    auto is_mut = false;
    if (this->current_token == TokenType::KW_MUT)
    {
        is_mut = true;
        this->get_new_token();
    }

    if (this->current_token != TokenType::IDENTIFIER)
        this->report_error(
            L"expected an identifier in a variable declaration");

    auto name = std::get<std::wstring>(this->current_token.value);
    auto type = this->parse_var_type();
    auto initial_value = this->parse_var_value();

    this->assert_current_and_eat(
        TokenType::SEMICOLON, L"no semicolon found in a variable declaration");

    return std::make_unique<VarDeclStmt>(name, type, initial_value, is_mut,
                                         this->pop_pos());
}

std::unique_ptr<ReturnStmt> Parser::parse_return_statement()
{
    this->push_pos();
    this->get_new_token();
    if (this->current_token == TokenType::SEMICOLON)
    {
        this->get_new_token();
        return std::make_unique<ReturnStmt>(this->pop_pos());
    }
    auto expr = this->parse_expression();
    this->assert_current_and_eat(TokenType::SEMICOLON,
                                 L"no semicolon found in a return statement");
    return std::make_unique<ReturnStmt>(expr, this->pop_pos());
}

std::unique_ptr<AssignStmt> Parser::parse_assign_statement()
{
    this->push_pos();
    auto name = std::get<std::wstring>(this->current_token.value);
    this->get_new_token();
    if (!this->assign_map.contains(this->current_token.type))
        this->report_error(L"invalid assignment operator");
    auto assign_type = this->assign_map.at(this->current_token.type);
    this->get_new_token();
    auto value = this->parse_expression();
    this->assert_current_and_eat(
        TokenType::SEMICOLON,
        L"no semicolon found in an assignment statement");
    return std::make_unique<AssignStmt>(name, assign_type, value,
                                        this->pop_pos());
}

std::unique_ptr<Statement> Parser::parse_block_statement()
{
    if (this->current_token == TokenType::KW_LET)
        return this->parse_variable_declaration();
    else if (this->current_token == TokenType::KW_RETURN)
        return this->parse_return_statement();
    else if (this->current_token == TokenType::L_BRACKET)
        return this->parse_block();
    else if (this->current_token == TokenType::IDENTIFIER)
        return this->parse_assign_statement();
    else
        return this->report_error(L"invalid token found in a block");
}

// Block = {Statement}
std::unique_ptr<Block> Parser::parse_block()
{
    std::vector<std::unique_ptr<Statement>> statements;
    this->push_pos();
    this->get_new_token();
    for (; this->current_token != TokenType::R_BRACKET;)
    {
        statements.push_back(this->parse_block_statement());
    }
    this->get_new_token();
    return std::make_unique<Block>(statements, this->pop_pos());
}

std::unique_ptr<ExprNode> Parser::parse_paren_expression()
{
    this->push_pos();
    this->get_new_token();
    auto expr = this->parse_expression();
    this->assert_current_and_eat(
        TokenType::R_PAREN, L"expected a right parenthesis in the expression");

    if (this->current_token != TokenType::L_PAREN)
        return expr;

    return this->parse_call_or_lambda(expr);
}

std::unique_ptr<ExprNode> Parser::parse_unary_expression()
{
    this->push_pos();
    auto op = this->unary_map.at(this->current_token.type);
    this->get_new_token();
    auto expr = this->parse_lhs();
    return std::make_unique<UnaryExpr>(expr, op, this->pop_pos());
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

std::unique_ptr<LambdaCallExpr> Parser::return_ellipsis_lambda(
    ExprNodePtr &expr, std::vector<std::unique_ptr<ExprNode>> &args,
    std::vector<std::optional<std::unique_ptr<ExprNode>>> &lambda_args,
    bool &is_lambda)
{

    convert_if_not_lambda(args, lambda_args, is_lambda);
    this->assert_next_token(TokenType::R_PAREN,
                            L"expected ')' after ellipsis in argument list");
    this->get_new_token();
    if (!lambda_args.empty() && lambda_args.back())
    {
        return std::make_unique<LambdaCallExpr>(expr, lambda_args, true,
                                                this->pop_pos());
    }
    return this->report_error(L"last in-place lambda argument before the "
                              L"ellipsis must not be a placeholder");
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

std::unique_ptr<LambdaCallExpr> Parser::handle_call_and_lambda_args(
    ExprNodePtr &expr, std::vector<ExprNodePtr> &args,
    std::vector<std::optional<ExprNodePtr>> &lambda_args, bool &is_lambda)
{
    for (;;)
    {
        switch (this->current_token.type)
        {
        case TokenType::PLACEHOLDER:
            this->handle_placeholder(args, lambda_args, is_lambda);
            break;
        case TokenType::ELLIPSIS:
            return this->return_ellipsis_lambda(expr, args, lambda_args,
                                                is_lambda);
        default:
            this->push_expr(args, lambda_args, is_lambda);
        }
        if (this->eat_comma_or_rparen())
            return nullptr;
    }
}

ExprNodePtr Parser::return_call_or_lambda(
    ExprNodePtr &expr, std::vector<ExprNodePtr> &args,
    std::vector<std::optional<ExprNodePtr>> &lambda_args,
    const bool &is_lambda)
{
    if (is_lambda)
        return std::make_unique<LambdaCallExpr>(expr, lambda_args, false,
                                                this->pop_pos());
    else
        return std::make_unique<CallExpr>(expr, args, this->pop_pos());
}

std::unique_ptr<ExprNode> Parser::parse_call_or_lambda(ExprNodePtr &expr)
{
    std::vector<ExprNodePtr> args;
    std::vector<std::optional<ExprNodePtr>> lambda_args;
    auto is_lambda = false;

    if (this->get_new_token() != TokenType::R_PAREN)
    {
        if (auto lambda = this->handle_call_and_lambda_args(
                expr, args, lambda_args, is_lambda))
        {
            return lambda;
        }
    }
    else
        this->get_new_token();
    auto result = return_call_or_lambda(expr, args, lambda_args, is_lambda);

    if (this->current_token != TokenType::L_PAREN)
        return result;

    return this->parse_call_or_lambda(result);
}

std::unique_ptr<ExprNode> Parser::parse_identifier_expression()
{
    auto name = std::get<std::wstring>(this->current_token.value);
    this->push_pos();
    this->get_new_token();
    ExprNodePtr expr = std::make_unique<VariableExpr>(name, this->read_pos());

    if (this->current_token != TokenType::L_PAREN)
    {
        this->pop_pos();
        return expr;
    }

    return this->parse_call_or_lambda(expr);
}

std::unique_ptr<ExprNode> Parser::parse_lhs()
{
    if (this->current_token == TokenType::IDENTIFIER)
        return this->parse_identifier_expression();
    else if (this->current_token == TokenType::L_PAREN)
        return this->parse_paren_expression();
    else if (this->unary_map.contains(this->current_token.type))
        return this->parse_unary_expression();
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
        lhs = std::make_unique<BinaryExpr>(lhs, rhs, op, this->pop_pos());
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

// Program = {VarDeclStmt | FuncDefStmt | ExternStmt}
ProgramPtr Parser::parse()
{
    std::vector<std::unique_ptr<VarDeclStmt>> globals;
    std::vector<std::unique_ptr<FuncDefStmt>> functions;
    std::vector<std::unique_ptr<ExternStmt>> externs;

    while (this->current_token)
    {
        if (auto func = this->parse_function())
            functions.push_back(func);
        if (auto ext = this->parse_extern())
            externs.push_back(ext);
        if (auto var = this->parse_variable_declaration())
            globals.push_back(var);
    }
    return std::make_unique<Program>(globals, functions, externs);
}
