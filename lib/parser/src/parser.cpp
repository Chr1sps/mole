#include "parser.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <optional>
#include <stack>
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
    UNOP(INCREMENT, INC),   UNOP(DECREMENT, DEC), UNOP(NEG, NEG),
    UNOP(BIT_NEG, BIT_NEG), UNOP(MINUS, MINUS),
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
    do
    {
        this->current_token = this->lexer->get_token();
    } while (this->current_token == TokenType::COMMENT);
}

void Parser::assert_current_and_eat(TokenType type,
                                    const std::wstring &error_msg)
{
    if (this->current_token != type)
        this->report_error(error_msg);
    this->next_token();
}

void Parser::report_error(const std::wstring &msg)
{
    auto error_msg = build_wstring(
        L"[ERROR] Parser error at [", this->current_token->position.line, ",",
        this->current_token->position.column, "]: ", msg, ".");
    auto log_msg = LogMessage(error_msg, LogLevel::ERROR);
    for (auto logger : this->loggers)
    {
        logger->log(log_msg);
    }
}

//
//
//
// PROGRAM = {VAR_DECL_STMT | FUNC_DEF_STMT | EXTERN_STMT}
ProgramPtr Parser::parse()
{
    std::vector<std::unique_ptr<VarDeclStmt>> globals;
    std::vector<std::unique_ptr<FuncDefStmt>> functions;
    std::vector<std::unique_ptr<ExternStmt>> externs;

    while (this->current_token)
    {
        if (auto func = this->parse_func_def_stmt())
            functions.push_back(std::move(func));
        else if (auto ext = this->parse_extern_stmt())
            externs.push_back(std::move(ext));
        else if (auto var = this->parse_var_decl_stmt())
            globals.push_back(std::move(var));
        else
        {
            this->report_error(L"function definition, extern statement or "
                               L"variable declaration expected");
            return nullptr;
        }
    }
    return std::make_unique<Program>(globals, functions, externs);
}

// EXTERN_STMT = KW_EXTERN, FUNC_NAME_AND_PARAMS, SEMICOLON;
std::unique_ptr<ExternStmt> Parser::parse_extern_stmt()
{
    if (this->current_token != TokenType::KW_EXTERN)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();

    auto [name, params, return_type] = this->parse_func_name_and_params();

    this->assert_current_and_eat(TokenType::SEMICOLON,
                                 L"not a semicolon in an extern declaration");

    return std::make_unique<ExternStmt>(name, params, return_type, position);
}

// VAR_DECL_STMT = KW_LET, [KW_MUT], IDENTIFIER, [TYPE_SPECIFIER],
// [INITIAL_VALUE];
std::unique_ptr<VarDeclStmt> Parser::parse_var_decl_stmt()
{
    if (this->current_token != TokenType::KW_LET)
        return nullptr;
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
    this->next_token();

    auto type = this->parse_type_specifier();
    auto initial_value = this->parse_initial_value();

    this->assert_current_and_eat(
        TokenType::SEMICOLON, L"no semicolon found in a variable declaration");

    return std::make_unique<VarDeclStmt>(name, type, initial_value, is_mut,
                                         position);
}

// TYPE_SPECIFIER = COLON, TYPE;
TypePtr Parser::parse_type_specifier()
{
    TypePtr type = nullptr;
    if (this->current_token == TokenType::COLON)
    {
        this->next_token();
        type = this->parse_type();
    }
    return type;
}

// INITIAL_VALUE = ASSIGN, BINARY_EXPR;
ExprNodePtr Parser::parse_initial_value()
{
    ExprNodePtr value = nullptr;
    if (this->current_token == TokenType::ASSIGN)
    {
        this->next_token();
        value = this->parse_binary_expr();
    }
    return value;
}

// FUNC_DEF_STMT = KW_FN, [KW_CONST], FUNC_NAME_AND_PARAMS, Block;
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
    this->next_token();

    this->assert_current_and_eat(
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
    ParamPtr param;
    if ((param = this->parse_parameter()))
    {
        params.push_back(std::move(param));
        while (this->current_token == TokenType::COMMA)
        {
            this->next_token();
            if ((param = this->parse_parameter()))
            {
                params.push_back(std::move(param));
            }
            else
            {
                this->report_error(L"expected a parameter definition after a "
                                   L"comma in a function definition");
            }
        }
    }
    return params;
}

// PARAMETER = IDENTIFIER, TYPE_SPECIFIER;
// TODO
ParamPtr Parser::parse_parameter()
{
    if (this->current_token != TokenType::IDENTIFIER)
        return nullptr;
    auto name = std::get<std::wstring>(this->current_token->value);
    auto position = this->current_token->position;
    this->next_token();
    auto type = this->parse_type_specifier();
    return std::make_unique<Parameter>(name, std::move(type), position);
}

// TYPE = FUNCTION_TYPE | SIMPLE_TYPE
TypePtr Parser::parse_type()
{
    if (auto type = this->parse_function_type())
        return type;
    else if (auto type = this->parse_simple_type())
        return type;
    this->report_error(L"invalid type syntax");
    return nullptr;
}

// RETURN_TYPE = LAMBDA_ARROW, TYPE
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

// SIMPLE_TYPE = TYPE_U32 | TYPE_I32 | TYPE_F64 | TYPE_BOOL | TYPE_CHAR |
// TYPE_STR;
TypePtr Parser::parse_simple_type()
{
    auto ref_spec = RefSpecifier::NON_REF;
    if (this->current_token == TokenType::AMPERSAND)
    {
        ref_spec = RefSpecifier::REF;
        this->next_token();
        if (this->current_token == TokenType::KW_MUT)
        {
            ref_spec = RefSpecifier::MUT_REF;
            this->next_token();
        }
    }
    if (this->type_map.contains(this->current_token->type))
    {
        auto result = std::make_unique<SimpleType>(
            this->type_map[this->current_token->type], ref_spec);
        this->next_token();
        return result;
    }
    else if (ref_spec != RefSpecifier::NON_REF)
        this->report_error(L"type name not found after a reference specifier");
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
            statements.push_back(std::move(stmt));
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
    if ((result = this->parse_return_stmt()))
        return result;
    if ((result = this->parse_assign_or_expr_stmt()))
        return result;
    if ((result = this->parse_var_decl_stmt()))
        return result;
    if ((result = this->parse_if_stmt()))
        return result;
    if ((result = this->parse_while_stmt()))
        return result;
    if ((result = this->parse_match_stmt()))
        return result;
    if ((result = this->parse_continue_stmt()))
        return result;
    if ((result = this->parse_break_stmt()))
        return result;
    if ((result = this->parse_block()))
        return result;
    return nullptr;
}

// RETURN_STMT = KW_RETURN, [BINARY_EXPR], SEMICOLON;
std::unique_ptr<ReturnStmt> Parser::parse_return_stmt()
{
    if (this->current_token != TokenType::KW_RETURN)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    if (this->current_token == TokenType::SEMICOLON)
    {
        this->next_token();
        return std::make_unique<ReturnStmt>(position);
    }
    auto expr = this->parse_binary_expr();
    this->assert_current_and_eat(TokenType::SEMICOLON,
                                 L"no semicolon found in a return statement");
    return std::make_unique<ReturnStmt>(expr, position);
}

// ASSIGN_OR_EXPR_STMT = BINARY_STMT, [ASSIGN_PART], SEMICOLON;
std::unique_ptr<Statement> Parser::parse_assign_or_expr_stmt()
{
    StmtPtr result = nullptr;
    if (auto lhs = this->parse_binary_expr())
    {
        if (auto op_and_rhs = this->parse_assign_part())
        {
            auto [op, rhs] = std::move(*op_and_rhs);
            result = std::make_unique<AssignStmt>(lhs, op, rhs, lhs->position);
        }
        else
            result = std::make_unique<ExprStmt>(lhs, lhs->position);
        this->assert_current_and_eat(
            TokenType::SEMICOLON,
            L"semicolon expected after an assignment or expression statement");
    }
    return result;
}

// ASSIGN_PART = ASSIGN_OP, BINARY_EXPR;
std::optional<std::tuple<AssignType, ExprNodePtr>> Parser::parse_assign_part()
{
    if (auto op = this->parse_assign_op())
    {
        auto rhs = this->parse_binary_expr();
        return std::tuple(*op, std::move(rhs));
    }
    return std::nullopt;
}

// ASSIGN_OP = ASSIGN | ASSIGN_PLUS | ASSIGN_MINUS | ASSIGN_STAR | ASSIGN_SLASH
// | ASSIGN_PERCENT | ASSIGN_EXP | ASSIGN_BIT_NEG | ASSIGN_AMPERSAND |
// ASSIGN_BIT_OR | ASSIGN_BIT_XOR | ASSIGN_SHIFT_LEFT | ASSIGN_SHIFT_RIGHT;
std::optional<AssignType> Parser::parse_assign_op()
{
    decltype(this->assign_map)::iterator iter;
    if (this->current_token.has_value() &&
        (iter = this->assign_map.find(this->current_token->type)) !=
            this->assign_map.end())
    {
        this->next_token();
        return iter->second;
    }
    return std::nullopt;
}

// CONTINUE_STMT = KW_CONTINUE, SEMICOLON;
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

// BREAK_STMT = KW_BREAK, SEMICOLON;
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

// IF_STMT = KW_IF, PAREN_EXPR, BLOCK, [ELSE_BLOCK];
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
    if (!matched_expr)
    {
        this->report_error(
            L"no parenthesis expression found in a match statement");
        return nullptr;
    }
    this->assert_current_and_eat(TokenType::L_BRACKET,
                                 L"no left bracket in a match statement");
    std::vector<MatchArmPtr> match_cases;
    while (true)
    {
        if (auto match_case = this->parse_match_arm())
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
    return std::make_unique<MatchStmt>(matched_expr, match_cases, position);
}

// MATCH_CASE = MATCH_SPECIFIER, LAMBDA_ARROW, BLOCK;
std::unique_ptr<MatchArm> Parser::parse_match_arm()
{
    if (auto arm = this->parse_literal_arm())
        return arm;
    if (auto arm = this->parse_guard_arm())
        return arm;
    if (auto arm = this->parse_placeholder_arm())
        return arm;
    return nullptr;
}

// LITERAL_ARM = LITERAL_CONDITION, MATCH_ARM_BLOCK;
std::unique_ptr<LiteralArm> Parser::parse_literal_arm()
{
    if (auto literals = this->parse_literal_condition())
    {
        auto [position, conditions] = std::move(*literals);
        auto block = this->parse_match_arm_block();
        return std::make_unique<LiteralArm>(conditions, block, position);
    }
    return nullptr;
}

// LITERAL_CONDITION = UNARY_EXPR, {BIT_OR, UNARY_EXPR};
std::optional<std::tuple<Position, std::vector<ExprNodePtr>>> Parser::
    parse_literal_condition()
{
    ExprNodePtr expr;
    if ((expr = this->parse_unary_expr()))
    {
        auto position = expr->position;
        std::vector<ExprNodePtr> conditions;
        conditions.push_back(std::move(expr));
        while (this->current_token == TokenType::BIT_OR)
        {
            this->next_token();
            if ((expr = this->parse_unary_expr()))
            {
                conditions.push_back(std::move(expr));
            }
            else
            {
                this->report_error(L"expected an expression in a literal "
                                   L"condition match specifier");
            }
        }

        return std::tuple(position, std::move(conditions));
    }
    else
        return std::nullopt;
}

// GUARD_ARM = GUARD_CONDITION, MATCH_ARM_BLOCK;
std::unique_ptr<GuardArm> Parser::parse_guard_arm()
{
    if (auto guard = this->parse_guard_condition())
    {
        auto block = this->parse_match_arm_block();
        return std::make_unique<GuardArm>(guard, block, guard->position);
    }
    return nullptr;
}

// GUARD_CONDITION = KW_IF, PAREN_EXPR;
ExprNodePtr Parser::parse_guard_condition()
{
    if (this->current_token != TokenType::KW_IF)
        return nullptr;
    this->next_token();
    return this->parse_paren_expr();
}

// PLACEHOLDER_ARM = PLACEHOLDER, MATCH_ARM_BLOCK;
std::unique_ptr<PlaceholderArm> Parser::parse_placeholder_arm()
{
    if (this->current_token != TokenType::PLACEHOLDER)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    auto block = parse_match_arm_block();
    return std::make_unique<PlaceholderArm>(block, position);
}

// MATCH_ARM_BLOCK = LAMBDA_ARROW, BLOCK;
BlockPtr Parser::parse_match_arm_block()
{
    if (this->current_token != TokenType::LAMBDA_ARROW)
    {
        this->report_error(L"no match case block found after the condition");
        return nullptr;
    }
    this->next_token();
    return this->parse_block();
}

// VARIABLE_EXPR = IDENTIFIER;
std::unique_ptr<VariableExpr> Parser::parse_variable_expr()
{
    if (this->current_token != TokenType::IDENTIFIER)
        return nullptr;
    auto result = std::make_unique<VariableExpr>(
        std::get<std::wstring>(this->current_token->value),
        this->current_token->position);
    this->next_token();
    return result;
}

// F64 = DOUBLE;
std::unique_ptr<F64Expr> Parser::parse_f64_expr()
{
    if (this->current_token != TokenType::DOUBLE)
        return nullptr;
    auto result =
        std::make_unique<F64Expr>(std::get<double>(this->current_token->value),
                                  this->current_token->position);
    this->next_token();
    return result;
}

// U32_EXPR = INT;
std::unique_ptr<I32Expr> Parser::parse_u32_expr()
{
    if (this->current_token != TokenType::INT)
        return nullptr;
    auto result = std::make_unique<I32Expr>(
        std::get<unsigned long long>(this->current_token->value),
        this->current_token->position);
    this->next_token();
    return result;
}

// STRING_EXPR = STRING;
std::unique_ptr<StringExpr> Parser::parse_string_expr()
{
    if (this->current_token != TokenType::STRING)
        return nullptr;
    auto result = std::make_unique<StringExpr>(
        std::get<std::wstring>(this->current_token->value),
        this->current_token->position);
    this->next_token();
    return result;
}

// CHAR_EXPR = CHAR;
std::unique_ptr<CharExpr> Parser::parse_char_expr()
{
    if (this->current_token != TokenType::CHAR)
        return nullptr;
    auto result = std::make_unique<CharExpr>(
        std::get<wchar_t>(this->current_token->value),
        this->current_token->position);
    this->next_token();
    return result;
}

// BOOL_EXPR = KW_FALSE | KW_TRUE;
std::unique_ptr<BoolExpr> Parser::parse_bool_expr()
{
    if (this->current_token == TokenType::KW_TRUE ||
        this->current_token == TokenType::KW_FALSE)
    {
        auto value = this->current_token == TokenType::KW_TRUE;
        auto position = this->current_token->position;
        auto result = std::make_unique<BoolExpr>(value, position);
        this->next_token();
        return result;
    }
    else
        return nullptr;
}

bool join_into_binary_op(std::stack<ExprNodePtr> &values,
                         std::stack<BinOpData> &ops)
{
    auto op = ops.top().type;
    ops.pop();
    if (values.size() < 2)
    {
        return false;
    }
    else
    {
        // moving the nodes out of the values stack - the pointers
        // stored in it must release the ownership
        auto rhs = std::move(values.top());
        values.pop();

        auto lhs = std::move(values.top());
        values.pop();

        auto position = lhs->position;
        auto new_expr = std::make_unique<BinaryExpr>(lhs, rhs, op, position);
        values.push(std::move(new_expr));
        return true;
    }
}

// BINARY_EXPR = CAST_EXPR, {BINARY_OP, CAST_EXPR};
ExprNodePtr Parser::parse_binary_expr()
{
    std::stack<ExprNodePtr> values;
    std::stack<BinOpData> ops;
    decltype(this->binary_map)::iterator iter;
    while (true)
    {
        if (auto expr = this->parse_cast_expr())
        {
            values.push(std::move(expr));
        }
        else if (this->current_token.has_value() &&
                 (iter = this->binary_map.find(this->current_token->type)) !=
                     this->binary_map.end())
        {
            while (!ops.empty() &&
                   (iter->second.precedence < ops.top().precedence ||
                    (iter->second.precedence == ops.top().precedence &&
                     !iter->second.is_right_assoc)))
            {
                if (!join_into_binary_op(values, ops))
                {
                    this->report_error(
                        L"no right-hand side found in a binary expression");
                }
            }
            ops.push(iter->second);
            this->next_token();
        }
        else
            break;
    }
    while (!ops.empty())
    {
        if (!join_into_binary_op(values, ops))
        {
            this->report_error(
                L"no right-hand side found in a binary expression");
        }
    }
    if (values.size() > 1)
    {
        this->report_error(L"binary expression doesn't have an operand");
        return nullptr;
    }
    else if (values.empty())
    {
        return nullptr;
    }
    auto result = std::move(values.top());
    values.pop();
    return result;
}

// CAST_EXPR = UNARY_EXPR , {KW_AS, SIMPLE_TYPE};
ExprNodePtr Parser::parse_cast_expr()
{
    auto lhs = this->parse_unary_expr();
    while (this->current_token == TokenType::KW_AS)
    {
        auto position = lhs->position;
        this->next_token();
        if (auto type = this->parse_simple_type())
        {
            lhs = std::make_unique<CastExpr>(lhs, type, position);
        }
        else
        {
            this->report_error(L"no type name given in a cast expression");
        }
    }
    return lhs;
}

// UNARY_EXPR = {UNARY_OP}, (INDEX_LAMBDA_OR_CALL);
ExprNodePtr Parser::parse_unary_expr()
{
    std::stack<UnaryOpEnum> ops;
    std::stack<Position> positions;
    decltype(this->unary_map)::iterator op_iter;
    while (this->current_token.has_value() &&
           (op_iter = this->unary_map.find(this->current_token->type)) !=
               this->unary_map.end())
    {
        ops.push(op_iter->second);
        positions.push(this->current_token->position);
        this->next_token();
    }
    if (auto inner = this->parse_index_lambda_or_call())
    {
        while (!ops.empty())
        {
            inner =
                std::make_unique<UnaryExpr>(inner, ops.top(), positions.top());
            ops.pop();
            positions.pop();
        }
        return inner;
    }
    else if (!ops.empty())
    {
        this->report_error(L"no inner expression found in a unary expression");
    }
    return nullptr;
}

// INDEX_LAMBDA_OR_CALL = FACTOR, {CALL_PART | LAMBDA_CALL_PART | INDEX_PART};
ExprNodePtr Parser::parse_index_lambda_or_call()
{
    auto result = this->parse_factor();
    while (true)
    {
        if (auto params = this->parse_call_part())
        {
            result =
                std::make_unique<CallExpr>(result, *params, result->position);
        }
        else if (auto params = this->parse_lambda_call_part())
        {
            result = std::make_unique<LambdaCallExpr>(result, *params,
                                                      result->position);
        }
        else if (auto param = this->parse_index_part())
        {
            result =
                std::make_unique<IndexExpr>(result, param, result->position);
        }
        else
            break;
    }
    return result;
}

// CALL_PART = L_PAREN, [ARGS], R_PAREN;
std::optional<std::vector<ExprNodePtr>> Parser::parse_call_part()
{
    if (this->current_token != TokenType::L_PAREN)
        return std::nullopt;
    this->next_token();
    auto args = this->parse_args();
    this->assert_current_and_eat(
        TokenType::R_PAREN, L"expected right parenthesis in call expression");
    return args;
}

// ARGS = BINARY_EXPR, {COMMA, BINARY_EXPR};
std::vector<ExprNodePtr> Parser::parse_args()
{
    ExprNodePtr arg;
    std::vector<ExprNodePtr> args;
    if ((arg = this->parse_binary_expr()))
    {
        args.push_back(std::move(arg));
        while (this->current_token == TokenType::COMMA)
        {
            this->next_token();
            if ((arg = this->parse_binary_expr()))
            {
                args.push_back(std::move(arg));
            }
            else
            {
                this->report_error(L"argument expected after a comma in a "
                                   L"function call argument list");
            }
        }
    }
    return args;
}

// LAMBDA_CALL_PART = AT, L_PAREN, [LAMBDA_ARGS], R_PAREN;
std::optional<std::vector<ExprNodePtr>> Parser::parse_lambda_call_part()
{
    if (this->current_token != TokenType::AT)
        return std::nullopt;
    this->next_token();
    this->assert_current_and_eat(
        TokenType::L_PAREN,
        L"expected left parenthesis in lambda call expression");
    auto args = this->parse_lambda_args();
    this->assert_current_and_eat(
        TokenType::R_PAREN,
        L"expected right parenthesis in lambda call expression");
    return args;
}

// LAMBDA_ARGS = LAMBDA_ARG, {COMMA, LAMBDA_ARG};
std::vector<ExprNodePtr> Parser::parse_lambda_args()
{
    std::optional<ExprNodePtr> arg;
    std::vector<ExprNodePtr> args;
    if ((arg = this->parse_lambda_arg()))
    {
        args.push_back(std::move(*arg));
        while (this->current_token == TokenType::COMMA)
        {
            this->next_token();
            if ((arg = this->parse_lambda_arg()))
            {
                args.push_back(std::move(*arg));
            }
            else
            {
                this->report_error(
                    L"argument or placeholder expected after a comma in a "
                    L"lambda call argument list");
            }
        }
    }
    return args;
}

// LAMBDA_ARG = BINARY_EXPR | PLACEHOLDER;
std::optional<ExprNodePtr> Parser::parse_lambda_arg()
{
    if (auto arg = this->parse_binary_expr())
        return arg;
    else if (this->current_token == TokenType::PLACEHOLDER)
    {
        this->next_token();
        return nullptr;
    }
    else
        return std::nullopt;
}

// INDEX_PART = L_SQ_BRACKET, BINARY_EXPR, R_SQ_BRACKET;
ExprNodePtr Parser::parse_index_part()
{
    if (this->current_token != TokenType::L_SQ_BRACKET)
        return nullptr;
    auto result = this->parse_binary_expr();
    this->assert_current_and_eat(
        TokenType::R_SQ_BRACKET,
        L"expected right square bracket in an index expression");
    return result;
}

// FACTOR = PAREN_EXPR | U32_EXPR | F64_EXPR | STRING_EXPR | CHAR_EXPR |
// BOOL_EXPR;
ExprNodePtr Parser::parse_factor()
{
    if (auto result = this->parse_u32_expr())
        return result;
    else if (auto result = this->parse_f64_expr())
        return result;
    else if (auto result = this->parse_string_expr())
        return result;
    else if (auto result = this->parse_char_expr())
        return result;
    else if (auto result = this->parse_bool_expr())
        return result;
    else if (auto result = this->parse_paren_expr())
        return result;
    else if (auto result = this->parse_variable_expr())
        return result;
    return nullptr;
}

// PAREN_EXPR = L_PAREN, BINARY_EXPR, R_PAREN;
ExprNodePtr Parser::parse_paren_expr()
{
    if (this->current_token != TokenType::L_PAREN)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();

    auto result = this->parse_binary_expr();
    if (!result)
    {
        this->report_error(L"no expression found after a left parenthesis");
        return result;
    }

    this->assert_current_and_eat(
        TokenType::R_PAREN,
        L"expected a right bracket in a parenthesis expression");

    result->position = position;
    return result;
}

void Parser::add_logger(Logger *logger)
{
    this->loggers.insert(logger);
}

void Parser::remove_logger(Logger *logger)
{
    this->loggers.erase(logger);
}

LexerPtr Parser::attach_lexer(LexerPtr &lexer)
{
    auto temp = std::move(this->lexer);
    this->lexer = std::move(lexer);
    this->next_token();
    return temp;
}

LexerPtr Parser::detach_lexer()
{
    auto temp = std::move(this->lexer);
    this->lexer = nullptr;
    return temp;
}

bool Parser::is_lexer_attached() const
{
    return this->lexer != nullptr;
}