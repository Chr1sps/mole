#include "parser.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <optional>
#include <tuple>

#define BINOP(token_type, precedence, bin_op_type, is_right)                  \
    {                                                                         \
        TokenType::token_type,                                                \
            BinOpData(BinOpEnum::bin_op_type, precedence, is_right)           \
    }
std::map<TokenType, BinOpData> Parser::binary_map{
    BINOP(EXP, 55, EXP, true),            // `^^` - 55
    BINOP(STAR, 50, MUL, false),          // `*`, `/`, `%` - 50
    BINOP(SLASH, 50, DIV, false),         // |
    BINOP(PERCENT, 50, MOD, false),       // |
    BINOP(PLUS, 45, ADD, false),          // `+`, `-` - 45
    BINOP(MINUS, 45, SUB, false),         // |
    BINOP(SHIFT_LEFT, 40, SHL, false),    // `<<`, `>>` - 40
    BINOP(SHIFT_RIGHT, 40, SHR, false),   // |
    BINOP(AMPERSAND, 35, BIT_AND, false), // `&` - 35
    BINOP(BIT_XOR, 30, BIT_XOR, false),   // `^` - 30
    BINOP(BIT_OR, 25, BIT_OR, false),     // `|` - 25
    BINOP(EQUAL, 20, EQ, false),          // `==`,`!=`,`>`,`>=`,`<`,`<=` - 20
    BINOP(NOT_EQUAL, 20, NEQ, false),     // |
    BINOP(GREATER, 20, GT, false),        // |
    BINOP(GREATER_EQUAL, 20, GE, false),  // |
    BINOP(LESS, 20, LT, false),           // |
    BINOP(LESS_EQUAL, 20, LE, false),     // |
    BINOP(AND, 15, AND, false),           // `&&` - 15
    BINOP(OR, 10, OR, false),             // `||` - 10
};
#undef BINOP

#define UNOP(token_type, un_op_type)                                          \
    {                                                                         \
        TokenType::token_type, UnaryOpEnum::un_op_type                        \
    }
std::map<TokenType, UnaryOpEnum> Parser::unary_map{
    UNOP(INCREMENT, INC),
    UNOP(DECREMENT, DEC),
    UNOP(NEG, NEG),
    UNOP(BIT_NEG, BIT_NEG),
};
#undef UNOP

#define TYPE(token_type, type)                                                \
    {                                                                         \
        TokenType::token_type, TypeEnum::type                                 \
    }
std::map<TokenType, TypeEnum> Parser::type_map{
    TYPE(TYPE_I32, I32), TYPE(TYPE_U32, U32),   TYPE(TYPE_CHAR, CHAR),
    TYPE(TYPE_F64, F64), TYPE(TYPE_BOOL, BOOL), TYPE(TYPE_STR, STR),
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

void Parser::next_token()
{
    this->current_token = this->lexer->get_token();
}

void Parser::assert_current_and_eat(TokenType type,
                                    const std::wstring &error_msg)
{
    if (this->current_token != type)
        this->report_error(error_msg);
    this->next_token();
}

void Parser::assert_next_token(TokenType type, const std::wstring &error_msg)
{
    this->next_token();
    if (this->current_token != type)
        this->report_error(error_msg);
}

// I32 = INT
std::unique_ptr<I32Expr> Parser::parse_i32()
{
    auto result = std::make_unique<I32Expr>(
        std::get<unsigned long long>(this->current_token->value),
        this->current_token->position);
    this->next_token();
    return result;
}

// F64 = DOUBLE
std::unique_ptr<F64Expr> Parser::parse_f64()
{
    auto result =
        std::make_unique<F64Expr>(std::get<double>(this->current_token->value),
                                  this->current_token->position);
    this->next_token();
    return result;
}

std::optional<TypePtr> Parser::parse_var_type()
{
    std::optional<TypePtr> type = {};
    if (this->next_token() == TokenType::COLON)
    {
        this->next_token();
        type = this->parse_type();
    }
    return type;
}

std::optional<ExprNodePtr> Parser::parse_var_value()
{
    std::optional<ExprNodePtr> value = {};
    if (this->current_token == TokenType::ASSIGN)
    {
        this->next_token();
        value = this->parse_expression();
    }
    return value;
}

// VarDeclStmt = KW_LET, IDENTIFIER, [COLON, Type], [ASSIGN, Expression];
std::unique_ptr<VarDeclStmt> Parser::parse_var_decl_stmt()
{
    auto position = this->current_token->position;
    this->next_token();
    auto is_mut = false;
    if (this->current_token == TokenType::KW_MUT)
    {
        is_mut = true;
        this->next_token();
    }

    if (this->current_token != TokenType::IDENTIFIER)
        this->report_error(
            L"expected an identifier in a variable declaration");

    auto name = std::get<std::wstring>(this->current_token->value);
    auto type = this->parse_var_type();
    auto initial_value = this->parse_var_value();

    this->assert_current_and_eat(
        TokenType::SEMICOLON, L"no semicolon found in a variable declaration");

    return std::make_unique<VarDeclStmt>(name, type, initial_value, is_mut,
                                         position);
}

std::unique_ptr<AssignStmt> Parser::parse_assign_statement()
{
    auto position = this->current_token->position;
    auto name = std::get<std::wstring>(this->current_token->value);
    this->next_token();
    if (!this->assign_map.contains(this->current_token->type))
        this->report_error(L"invalid assignment operator");
    auto assign_type = this->assign_map.at(this->current_token->type);
    this->next_token();
    auto value = this->parse_expression();
    this->assert_current_and_eat(
        TokenType::SEMICOLON,
        L"no semicolon found in an assignment statement");
    return std::make_unique<AssignStmt>(name, assign_type, value, position);
}

std::unique_ptr<ExprNode> Parser::parse_paren_expr()
{
    this->next_token();
    auto expr = this->parse_expression();
    this->assert_current_and_eat(
        TokenType::R_PAREN, L"expected a right parenthesis in the expression");

    if (this->current_token != TokenType::L_PAREN)
        return expr;

    return this->parse_call_or_lambda(expr);
}

std::unique_ptr<ExprNode> Parser::parse_unary_expression()
{
    auto position = this->current_token->position;
    auto op = this->unary_map.at(this->current_token->type);
    this->next_token();
    auto expr = this->parse_lhs();
    return std::make_unique<UnaryExpr>(expr, op, position);
}

std::unique_ptr<ExprNode> Parser::parse_const_expression()
{
    switch (this->current_token->type)
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
    if (this->next_token() != TokenType::R_PAREN)
    {
        for (;;)
        {
            args.push_back(this->parse_expression());
            if (this->current_token == TokenType::R_PAREN)
            {
                this->next_token();
                break;
            }
            this->assert_current_and_eat(
                TokenType::COMMA, L"expected ')' or ',' in argument list");
        }
    }
    return args;
}

bool Parser::eat_comma_or_rparen()
{
    if (this->current_token == TokenType::R_PAREN)
    {
        this->next_token();
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

std::unique_ptr<ExprNode> Parser::parse_identifier_expression()
{
    auto name = std::get<std::wstring>(this->current_token->value);
    auto position = this->current_token->position;
    this->next_token();
    ExprNodePtr expr = std::make_unique<VariableExpr>(name, position);

    if (this->current_token != TokenType::L_PAREN)
    {
        return expr;
    }

    return this->parse_call_or_lambda(expr);
}

std::unique_ptr<ExprNode> Parser::parse_lhs()
{
    if (this->current_token == TokenType::IDENTIFIER)
        return this->parse_identifier_expression();
    else if (this->current_token == TokenType::L_PAREN)
        return this->parse_paren_expr();
    else if (this->unary_map.contains(this->current_token->type))
        return this->parse_unary_expression();
    else if (this->type_value_map.contains(this->current_token->type))
        return this->parse_const_expression();
    else
        return this->report_error(
            L"unknown token when expecting an expression");
}

std::unique_ptr<ExprNode> Parser::parse_expression()
{
    auto lhs = this->parse_lhs();
    if (this->binary_map.contains(this->current_token->type))
        return this->parse_op_and_rhs(0, lhs);
    return lhs;
}

std::nullptr_t Parser::report_error(const std::wstring &msg)
{
    auto error_msg = build_wstring(
        L"[ERROR] Parser error at [", this->current_token->position.line, ",",
        this->current_token->position.column, "]: ", msg, ".");
    throw std::exception();
    // throw std::runtime_error(error_msg.c_str());
    return nullptr;
}

//
//
//
// Program = {VarDeclStmt | FuncDefStmt | ExternStmt}
ProgramPtr Parser::parse()
{
    std::vector<std::unique_ptr<VarDeclStmt>> globals;
    std::vector<std::unique_ptr<FuncDefStmt>> functions;
    std::vector<std::unique_ptr<ExternStmt>> externs;

    while (this->current_token)
    {
        if (auto func = this->parse_func_def_stmt())
            functions.push_back(std::move(func));
        if (auto ext = this->parse_extern_stmt())
            externs.push_back(std::move(ext));
        if (auto var = this->parse_var_decl_stmt())
            globals.push_back(std::move(var));
    }
    return std::make_unique<Program>(globals, functions, externs);
}

// EXTERN_STMT = KW_EXTERN, FUNC_NAME_AND_PARAMS, SEMICOLON;
std::unique_ptr<ExternStmt> Parser::parse_extern_stmt()
{
    if (this->current_token != TokenType::KW_EXTERN)
        return nullptr;
    auto position = this->current_token->position;

    auto [name, params, return_type] = this->parse_func_name_and_params();

    this->assert_current_and_eat(TokenType::SEMICOLON,
                                 L"not a semicolon in an extern declaration");

    return std::make_unique<ExternStmt>(name, params, return_type, position);
}

// @brief FUNC_DEF_STMT = KW_FN, [KW_CONST], FUNC_NAME_AND_PARAMS, Block;
std::unique_ptr<FuncDefStmt> Parser::parse_func_def_stmt()
{
    if (this->current_token != TokenType::KW_FN)
        return nullptr;

    auto position = this->current_token->position;
    this->next_token();

    auto is_const = false;
    if (this->current_token == TokenType::KW_CONST)
    {
        is_const = true;
        this->next_token();
    }

    auto [name, params, return_type] = this->parse_func_name_and_params();
    auto block = this->parse_block();
    return std::make_unique<FuncDefStmt>(name, params, return_type, block,
                                         is_const, position);
}

// FUNC_NAME_AND_PARAMS = IDENTIFIER, L_PAREN, [PARAMS], R_PAREN,
// [RETURN_TYPE];
std::tuple<std::wstring, std::vector<ParamPtr>, TypePtr> Parser::
    parse_func_name_and_params()
{

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
    return {std::move(name), std::move(params), std::move(return_type)};
}

// Params = Parameter, {COMMA, Parameter}
std::vector<ParamPtr> Parser::parse_params()
{
    std::vector<ParamPtr> params;
    for (this->next_token(); this->current_token == TokenType::IDENTIFIER;)
    {
        auto parameter = this->parse_parameter();
        params.push_back(std::move(parameter));
        if (this->current_token == TokenType::COMMA)
            this->next_token();
    }
    return params;
}

// PARAMETER = IDENTIFIER, TYPE_SPECIFIER;
// TODO
ParamPtr Parser::parse_parameter()
{
    auto name = std::get<std::wstring>(this->current_token->value);
    this->assert_next_token(TokenType::COLON,
                            L"not a colon in a parameter expression");
    this->next_token();
    auto param_type = this->parse_type();
    return std::make_unique<Parameter>(name, std::move(param_type));
}

// TODO: TYPE_SPECIFIER
// TODO: TYPE_WITH_REF_SPEC
// TODO: REF_SPECIFIER

// RETURN_TYPE = LAMBDA_ARROW, TYPE_WITH_REF_SPEC
// TODO
TypePtr Parser::parse_return_type()
{
    TypePtr return_type = nullptr;
    if (this->current_token == TokenType::LAMBDA_ARROW)
    {
        this->next_token();
        return_type = this->parse_type();
    }
    return return_type;
}

// TYPE = FUNCTION_TYPE | SIMPLE_TYPE
TypePtr Parser::parse_type()
{
    if (auto type = this->parse_function_type())
        return type;
    else if (auto type = this->parse_simple_type())
        return type;
    return this->report_error(L"invalid type syntax");
}

// SIMPLE_TYPE = TYPE_U32 | TYPE_I32 | TYPE_F64 | TYPE_BOOL | TYPE_CHAR |
// TYPE_STR;
TypePtr Parser::parse_simple_type()
{
    if (this->type_map.contains(this->current_token->type))
    {
        auto result = std::make_unique<SimpleType>(
            this->type_map[this->current_token->type]);
        this->next_token();
        return result;
    }
    return nullptr;
}

// FUNCTION_TYPE = KW_FN, L_PAREN, [TYPES], R_PAREN, [RETURN_TYPE]
std::unique_ptr<FunctionType> Parser::parse_function_type()
{
    if (this->current_token != TokenType::KW_FN)
        return nullptr;
    auto is_const = false;
    this->next_token();
    if (this->current_token == TokenType::KW_CONST)
    {
        is_const = true;
        this->next_token();
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

// TYPES = TYPE_WITH_REF_SPEC, {COMMA, TYPE_WITH_REF_SPEC}
// TODO
std::vector<TypePtr> Parser::parse_types()
{
    std::vector<TypePtr> types;
    for (; this->current_token != TokenType::R_PAREN; this->next_token())
    {
        auto type = this->parse_type();
        types.push_back(std::move(type));
        if (this->current_token != TokenType::COMMA)
            break;
    }
    return types;
}

// BLOCK = L_BRACKET, {NON_FUNC_STMT}, R_BRACKET;
std::unique_ptr<Block> Parser::parse_block()
{
    if (this->current_token != TokenType::L_BRACKET)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();

    std::vector<std::unique_ptr<Statement>> statements;
    while (true)
    {
        if (auto stmt = this->parse_non_func_stmt())
            statements.push_back(stmt);
        else
            break;
    }

    this->assert_current_and_eat(TokenType::R_BRACKET,
                                 L"block statement missing a right bracket");
    return std::make_unique<Block>(statements, position);
}

// NON_FUNC_STMT = RETURN_STMT | ASSIGN_STMT | VAR_DECL_STMT | IF_STMT |
// WHILE_STMT | MATCH_STMT | CONTINUE_STMT | BREAK_STMT | BLOCK;
std::unique_ptr<Statement> Parser::parse_non_func_stmt()
{
    std::unique_ptr<Statement> result;
    if (result = this->parse_return_stmt())
        return result;
    if (result = this->parse_assign_statement())
        return result;
    if (result = this->parse_var_decl_stmt())
        return result;
    if (result = this->parse_if_stmt())
        return result;
    if (result = this->parse_while_stmt())
        return result;
    if (result = this->parse_match_stmt())
        return result;
    if (result = this->parse_continue_stmt())
        return result;
    if (result = this->parse_break_stmt())
        return result;
    if (result = this->parse_block())
        return result;
}

// RETURN_STMT = KW_RETURN, [EXPRESSION], SEMICOLON;
std::unique_ptr<ReturnStmt> Parser::parse_return_stmt()
{
    auto position = this->current_token->position;
    this->next_token();
    if (this->current_token == TokenType::SEMICOLON)
    {
        this->next_token();
        return std::make_unique<ReturnStmt>(position);
    }
    auto expr = this->parse_expression();
    this->assert_current_and_eat(TokenType::SEMICOLON,
                                 L"no semicolon found in a return statement");
    return std::make_unique<ReturnStmt>(expr, position);
}

std::unique_ptr<ContinueStmt> Parser::parse_continue_stmt()
{
    if (this->current_token != TokenType::KW_CONTINUE)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    this->assert_current_and_eat(
        TokenType::SEMICOLON, L"no semicolon found in a continue statement");
    return std::make_unique<ContinueStmt>(position);
}

std::unique_ptr<BreakStmt> Parser::parse_break_stmt()
{
    if (this->current_token != TokenType::KW_BREAK)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    this->assert_current_and_eat(
        TokenType::SEMICOLON, L"no semicolon found in a continue statement");
    return std::make_unique<BreakStmt>(position);
}

std::unique_ptr<IfStmt> Parser::parse_if_stmt()
{
    if (this->current_token != TokenType::KW_IF)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();

    auto condition_expr = this->parse_paren_expr();
    auto then_block = this->parse_block();
    auto else_block = this->parse_else_block();
    return std::make_unique<IfStmt>(condition_expr, then_block, else_block,
                                    position);
}

// ELSE_BLOCK = KW_ELSE, BLOCK;
std::unique_ptr<Block> Parser::parse_else_block()
{
    if (this->current_token != TokenType::KW_ELSE)
        return nullptr;
    this->next_token();
    auto result = this->parse_block();
    if (!result)
        this->report_error(L"no block present after else");
    return result;
}

// WHILE_STMT = KW_WHILE, PAREN_EXPR, BLOCK;
std::unique_ptr<WhileStmt> Parser::parse_while_stmt()
{
    if (this->current_token != TokenType::KW_WHILE)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    auto condition_expr = this->parse_paren_expr();
    auto block = this->parse_block();
    return std::make_unique<WhileStmt>(condition_expr, block, position);
}

// MATCH_STMT = KW_MATCH, PAREN_EXPR, L_BRACKET, {MATCH_CASE}, R_BRACKET;
std::unique_ptr<MatchStmt> Parser::parse_match_stmt()
{
    if (this->current_token != TokenType::KW_MATCH)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    auto matched_expr = this->parse_paren_expr();
    this->assert_current_and_eat(TokenType::L_BRACKET,
                                 L"no left bracket in a match statement");
    std::vector<MatchCase> match_cases;
    while (true)
    {
        if (auto match_case = this->parse_match_case())
        {
            match_cases.push_back(std::move(match_case));
        }
        else
        {
            break;
        }
    }
    this->assert_current_and_eat(TokenType::R_BRACKET,
                                 L"no left bracket in a match statement");
}

// MATCH_CASE = MATCH_SPECIFIER, LAMBDA_ARROW, BLOCK;
std::unique_ptr<MatchCase> Parser::parse_match_case()
{
}