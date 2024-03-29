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
    UNOP(NEG, NEG), UNOP(BIT_NEG, BIT_NEG), UNOP(MINUS, MINUS),
    UNOP(AMPERSAND, REF), UNOP(STAR, DEREF)};
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
    TYPE_VALUE(INT, U32),
    TYPE_VALUE(DOUBLE, F64),
};
#undef TYPE_VALUE

std::map<TokenType, std::optional<BinOpEnum>> Parser::assign_map{
    {TokenType::ASSIGN, std::nullopt},
    {TokenType::ASSIGN_PLUS, BinOpEnum::ADD},
    {TokenType::ASSIGN_MINUS, BinOpEnum::SUB},
    {TokenType::ASSIGN_STAR, BinOpEnum::MUL},
    {TokenType::ASSIGN_SLASH, BinOpEnum::DIV},
    {TokenType::ASSIGN_PERCENT, BinOpEnum::MOD},
    {TokenType::ASSIGN_EXP, BinOpEnum::EXP},
    {TokenType::ASSIGN_AMPERSAND, BinOpEnum::BIT_AND},
    {TokenType::ASSIGN_BIT_OR, BinOpEnum::BIT_OR},
    {TokenType::ASSIGN_BIT_XOR, BinOpEnum::BIT_XOR},
    {TokenType::ASSIGN_SHIFT_LEFT, BinOpEnum::SHL},
    {TokenType::ASSIGN_SHIFT_RIGHT, BinOpEnum::SHR},
};

void Parser::report_error(const std::wstring &msg)
{
    this->report(LogLevel::ERROR, L"Parser error at [",
                 this->current_token->position.line, ",",
                 this->current_token->position.column, "]: ", msg, ".");
    throw ParserException();
}

void Parser::next_token()
{
    do
    {
        this->current_token = this->lexer->get_token();
    } while (this->current_token == TokenType::COMMENT);
}

bool Parser::assert_current_and_eat(TokenType type,
                                    const std::wstring &error_msg)
{
    if (this->current_token != type)
    {
        this->report_error(error_msg);
        return false;
    }
    this->next_token();
    return true;
}

// PROGRAM = {VAR_DECL_STMT | FUNC_DEF_STMT | EXTERN_STMT}
ProgramPtr Parser::parse()
{
    std::vector<std::unique_ptr<VarDeclStmt>> globals;
    std::vector<std::unique_ptr<FuncDef>> functions;
    std::vector<std::unique_ptr<ExternDef>> externs;
    try
    {
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
        return std::make_unique<Program>(
            std::move(globals), std::move(functions), std::move(externs));
    }
    catch (const LexerException &)
    {
        return nullptr;
    }
    catch (const ParserException &)
    {
        return nullptr;
    }
}

// EXTERN_STMT = KW_EXTERN, FUNC_NAME_AND_PARAMS, SEMICOLON;
std::unique_ptr<ExternDef> Parser::parse_extern_stmt()
{
    if (this->current_token != TokenType::KW_EXTERN)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();

    if (this->current_token != TokenType::IDENTIFIER)
    {
        this->report_error(L"not a function identifier");
        return nullptr;
    }
    auto name = std::get<std::wstring>((*this->current_token).value);
    this->next_token();

    if (!this->assert_current_and_eat(
            TokenType::L_PAREN,
            L"left parenthesis missing in a function definition"))
        return nullptr;

    auto params = this->parse_types();
    if (!this->assert_current_and_eat(TokenType::R_PAREN,
                                      L"right parenthesis missing "
                                      "in a function "
                                      "definition"))
        return nullptr;

    auto return_type = this->parse_return_type();

    if (!this->assert_current_and_eat(
            TokenType::SEMICOLON, L"not a semicolon in an extern declaration"))
        return nullptr;

    return std::make_unique<ExternDef>(name, params, return_type, position);
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
    {
        this->report_error(
            L"expected an identifier in a variable declaration");
        return nullptr;
    }
    auto name = std::get<std::wstring>(this->current_token->value);
    this->next_token();

    auto type = this->parse_type_specifier();
    auto initial_value = this->parse_initial_value();

    if (!this->assert_current_and_eat(
            TokenType::SEMICOLON,
            L"no semicolon found in a variable declaration"))
        return nullptr;

    return std::make_unique<VarDeclStmt>(
        name, std::move(type), std::move(initial_value), is_mut, position);
}

// TYPE_SPECIFIER = COLON, TYPE;
std::optional<Type> Parser::parse_type_specifier()
{
    if (this->current_token == TokenType::COLON)
    {
        this->next_token();
        if (auto result = this->parse_type())
            return *result;
        else
        {
            this->report_error(
                L"expected a type definition in a variable type specifier");
            return std::nullopt;
        }
    }
    return std::nullopt;
}

// INITIAL_VALUE = ASSIGN, BINARY_EXPR;
ExprPtr Parser::parse_initial_value()
{
    ExprPtr value;
    if (this->current_token == TokenType::ASSIGN)
    {
        this->next_token();
        value = this->parse_binary_expr();
        if (!value)
        {
            this->report_error(L"no initial value read");
        }
    }
    return value;
}

// FUNC_DEF_STMT = KW_FN, [KW_CONST], FUNC_NAME_AND_PARAMS, Block;
std::unique_ptr<FuncDef> Parser::parse_func_def_stmt()
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

    if (this->current_token != TokenType::IDENTIFIER)
    {
        this->report_error(L"expected a function identifier");
        return nullptr;
    }
    auto name = std::get<std::wstring>((*this->current_token).value);
    this->next_token();

    if (!this->assert_current_and_eat(
            TokenType::L_PAREN,
            L"left parenthesis missing in a function definition"))
        return nullptr;
    auto params = this->parse_params();
    if (!this->assert_current_and_eat(TokenType::R_PAREN,
                                      L"right parenthesis missing "
                                      "in a function "
                                      "definition"))
        return nullptr;

    auto return_type = this->parse_return_type();
    auto block = this->parse_block();
    if (!block)
    {
        this->report_error(L"expected a block in a function definition");
        return nullptr;
    }
    return std::make_unique<FuncDef>(name, std::move(params),
                                     std::move(return_type), std::move(block),
                                     is_const, position);
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
ParamPtr Parser::parse_parameter()
{
    if (this->current_token != TokenType::IDENTIFIER)
        return nullptr;
    auto name = std::get<std::wstring>(this->current_token->value);
    auto position = this->current_token->position;
    this->next_token();
    auto type = this->parse_type_specifier();
    if (!type.has_value())
        return nullptr;
    return std::make_unique<Parameter>(name, *type, position);
}

// RETURN_TYPE = LAMBDA_ARROW, TYPE
std::optional<Type> Parser::parse_return_type()
{
    if (this->current_token == TokenType::LAMBDA_ARROW)
    {
        this->next_token();
        if (auto result = this->parse_type())
            return result;
        else
        {
            this->report_error(L"expected a return type in a function type");
            return std::nullopt;
        }
    }
    return std::nullopt;
}

// TYPE = TYPE_U32 | TYPE_I32 | TYPE_F64 | TYPE_BOOL | TYPE_CHAR |
// TYPE_STR;
std::optional<Type> Parser::parse_type()
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
        auto result =
            Type(this->type_map[this->current_token->type], ref_spec);
        this->next_token();
        return result;
    }
    else if (ref_spec != RefSpecifier::NON_REF)
        this->report_error(L"type name not found after a reference specifier");
    return std::nullopt;
}

std::vector<Type> Parser::parse_types()
{
    std::vector<Type> types;
    if (auto type = this->parse_type())
    {
        types.push_back(*type);
        while (this->current_token == TokenType::COMMA)
        {
            this->next_token();
            type = this->parse_type();
            if (type)
                types.push_back(*type);
        }
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
    while (auto stmt = this->parse_non_func_stmt())
    {
        statements.push_back(std::move(stmt));
    }

    if (!this->assert_current_and_eat(
            TokenType::R_BRACKET, L"block statement missing a right bracket"))
        return nullptr;
    return std::make_unique<Block>(std::move(statements), position);
}

// NON_FUNC_STMT = RETURN_STMT | ASSIGN_STMT | VAR_DECL_STMT | IF_STMT |
// WHILE_STMT | MATCH_STMT | CONTINUE_STMT | BREAK_STMT | BLOCK;
StmtPtr Parser::parse_non_func_stmt()
{
    if (auto result = this->parse_return_stmt())
        return result;
    if (auto result = this->parse_assign_or_expr_stmt())
        return result;
    if (auto result = this->parse_if_stmt())
        return result;
    if (auto result = this->parse_while_stmt())
        return result;
    if (auto result = this->parse_match_stmt())
        return result;
    if (auto result = this->parse_continue_stmt())
        return result;
    if (auto result = this->parse_break_stmt())
        return result;
    if (auto result = this->parse_block())
        return std::make_unique<Statement>(std::move(*result));
    if (auto result = this->parse_var_decl_stmt())
        return std::make_unique<Statement>(std::move(*result));
    return nullptr;
}

// RETURN_STMT = KW_RETURN, [BINARY_EXPR], SEMICOLON;
StmtPtr Parser::parse_return_stmt()
{
    if (this->current_token != TokenType::KW_RETURN)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    if (this->current_token == TokenType::SEMICOLON)
    {
        this->next_token();
        return std::make_unique<Statement>(ReturnStmt(position));
    }
    auto expr = this->parse_binary_expr();
    if (!this->assert_current_and_eat(
            TokenType::SEMICOLON, L"no semicolon found in a return statement"))
        return nullptr;
    return std::make_unique<Statement>(ReturnStmt(std::move(expr), position));
}

// ASSIGN_OR_EXPR_STMT = BINARY_STMT, [ASSIGN_PART], SEMICOLON;
StmtPtr Parser::parse_assign_or_expr_stmt()
{
    StmtPtr result = nullptr;
    ExprPtr lhs = this->parse_binary_expr();
    if (lhs)
    {
        if (auto op_and_rhs = this->parse_assign_part())
        {
            auto [op, rhs] = std::move(*op_and_rhs);
            auto position = get_position(*lhs);
            result = std::make_unique<Statement>(
                AssignStmt(std::move(lhs), op, std::move(rhs), position));
        }
        else
        {
            auto position = get_position(*lhs);
            result = std::make_unique<Statement>(
                ExprStmt(std::move(lhs), position));
        }
        if (!this->assert_current_and_eat(
                TokenType::SEMICOLON, L"semicolon expected after an "
                                      L"assignment or expression statement"))
            return nullptr;
    }
    return result;
}

// ASSIGN_PART = ASSIGN_OP, BINARY_EXPR;
std::optional<std::tuple<std::optional<BinOpEnum>, ExprPtr>> Parser::
    parse_assign_part()
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
std::optional<std::optional<BinOpEnum>> Parser::parse_assign_op()
{
    decltype(this->assign_map)::const_iterator iter;
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
StmtPtr Parser::parse_continue_stmt()
{
    if (this->current_token != TokenType::KW_CONTINUE)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    if (!this->assert_current_and_eat(
            TokenType::SEMICOLON,
            L"no semicolon found in a continue statement"))
        return nullptr;
    return std::make_unique<Statement>(ContinueStmt(position));
}

// BREAK_STMT = KW_BREAK, SEMICOLON;
StmtPtr Parser::parse_break_stmt()
{
    if (this->current_token != TokenType::KW_BREAK)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    if (!this->assert_current_and_eat(
            TokenType::SEMICOLON,
            L"no semicolon found in a continue statement"))
        return nullptr;
    return std::make_unique<Statement>(BreakStmt(position));
}

// IF_STMT = KW_IF, PAREN_EXPR, BLOCK, [ELSE_BLOCK];
StmtPtr Parser::parse_if_stmt()
{
    if (this->current_token != TokenType::KW_IF)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();

    auto condition = this->parse_paren_expr();
    if (!condition)
    {
        this->report_error(
            L"no condition expression found in an if statement");
        return nullptr;
    }
    auto then_stmt = this->parse_non_func_stmt();
    if (!then_stmt)
    {
        this->report_error(
            L"no positive condition statement found in the if statement");
        return nullptr;
    }
    auto else_stmt = this->parse_else_block();
    return std::make_unique<Statement>(IfStmt(std::move(condition),
                                              std::move(then_stmt),
                                              std::move(else_stmt), position));
}

// ELSE_BLOCK = KW_ELSE, BLOCK;
StmtPtr Parser::parse_else_block()
{
    if (this->current_token != TokenType::KW_ELSE)
        return nullptr;
    this->next_token();
    auto result = this->parse_non_func_stmt();
    if (!result)
        this->report_error(L"no block present after else");
    return result;
}

// WHILE_STMT = KW_WHILE, PAREN_EXPR, BLOCK;
StmtPtr Parser::parse_while_stmt()
{
    if (this->current_token != TokenType::KW_WHILE)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    auto condition_expr = this->parse_paren_expr();
    if (!condition_expr)
    {
        this->report_error(
            L"no condition expression found in the while loop statement");
        return nullptr;
    }
    auto statement = this->parse_non_func_stmt();
    if (!statement)
    {
        this->report_error(L"no block found in the while loop statement");
        return nullptr;
    }
    return std::make_unique<Statement>(
        WhileStmt(std::move(condition_expr), std::move(statement), position));
}

// MATCH_STMT = KW_MATCH, PAREN_EXPR, L_BRACKET, {MATCH_CASE}, R_BRACKET;
StmtPtr Parser::parse_match_stmt()
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
    if (!this->assert_current_and_eat(TokenType::L_BRACKET,
                                      L"no left bracket in a match statement"))
        return nullptr;
    std::vector<MatchArmPtr> match_cases;
    while (auto match_case = this->parse_match_arm())
    {
        match_cases.push_back(std::move(match_case));
    }
    if (!this->assert_current_and_eat(TokenType::R_BRACKET,
                                      L"no left bracket in a match statement"))
        return nullptr;
    return std::make_unique<Statement>(
        MatchStmt(std::move(matched_expr), std::move(match_cases), position));
}

// MATCH_CASE = MATCH_SPECIFIER, LAMBDA_ARROW, BLOCK;
MatchArmPtr Parser::parse_match_arm()
{
    if (auto arm = this->parse_literal_arm())
        return arm;
    if (auto arm = this->parse_guard_arm())
        return arm;
    if (auto arm = this->parse_else_arm())
        return arm;
    return nullptr;
}

// LITERAL_ARM = LITERAL_CONDITION, MATCH_ARM_BLOCK;
MatchArmPtr Parser::parse_literal_arm()
{
    if (auto literals = this->parse_literal_condition())
    {
        auto [position, conditions] = std::move(*literals);
        auto block = this->parse_match_arm_block();
        if (!block)
        {
            this->report_error(L"no block found in a literal guard arm");
            return nullptr;
        }
        return std::make_unique<MatchArm>(
            LiteralArm(std::move(conditions), std::move(block), position));
    }
    return nullptr;
}

// LITERAL_CONDITION = UNARY_EXPR, {BIT_OR, UNARY_EXPR};
std::optional<std::tuple<Position, std::vector<ExprPtr>>> Parser::
    parse_literal_condition()
{
    ExprPtr expr = this->parse_unary_expr();
    if (expr)
    {
        auto position = get_position(*expr);
        std::vector<ExprPtr> conditions;
        conditions.push_back(std::move(expr));
        while (this->current_token == TokenType::BIT_OR)
        {
            this->next_token();
            expr = this->parse_unary_expr();
            if (expr)
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
MatchArmPtr Parser::parse_guard_arm()
{
    if (auto guard = this->parse_guard_condition())
    {
        auto [position, condition] = std::move(*guard);
        auto block = this->parse_match_arm_block();
        if (!block)
        {
            this->report_error(L"no block found in a guard match arm");
        }
        return std::make_unique<MatchArm>(
            GuardArm(std::move(condition), std::move(block), position));
    }
    return nullptr;
}

// GUARD_CONDITION = KW_IF, PAREN_EXPR;
std::optional<std::tuple<Position, ExprPtr>> Parser::parse_guard_condition()
{
    if (this->current_token != TokenType::KW_IF)
        return std::nullopt;
    auto position = this->current_token->position;
    this->next_token();
    return std::tuple(position, this->parse_paren_expr());
}

// PLACEHOLDER_ARM = PLACEHOLDER, MATCH_ARM_BLOCK;
MatchArmPtr Parser::parse_else_arm()
{
    if (this->current_token != TokenType::KW_ELSE)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();
    auto block = parse_match_arm_block();
    if (!block)
    {
        this->report_error(L"no block found in an else arm");
        return nullptr;
    }
    return std::make_unique<MatchArm>(ElseArm(std::move(block), position));
}

// MATCH_ARM_BLOCK = LAMBDA_ARROW, BLOCK;
StmtPtr Parser::parse_match_arm_block()
{
    if (this->current_token != TokenType::LAMBDA_ARROW)
    {
        this->report_error(L"no match case block found after the condition");
        return nullptr;
    }
    this->next_token();
    return this->parse_non_func_stmt();
}

// VARIABLE_EXPR = IDENTIFIER;
ExprPtr Parser::parse_variable_or_call()
{
    if (this->current_token != TokenType::IDENTIFIER)
        return nullptr;
    auto name = std::get<std::wstring>(this->current_token->value);
    auto position = this->current_token->position;
    this->next_token();
    if (this->current_token == TokenType::L_PAREN)
    {
        return this->parse_call(name, position);
    }
    else
    {
        return std::make_unique<Expression>(VariableExpr(name, position));
    }
}

// F64 = DOUBLE;
ExprPtr Parser::parse_f64_expr()
{
    if (this->current_token != TokenType::DOUBLE)
        return nullptr;
    auto result = std::make_unique<Expression>(
        F64Expr(std::get<double>(this->current_token->value),
                this->current_token->position));
    this->next_token();
    return result;
}

// U32_EXPR = INT;
ExprPtr Parser::parse_u32_expr()
{
    if (this->current_token != TokenType::INT)
        return nullptr;
    auto result = std::make_unique<Expression>(
        U32Expr(std::get<unsigned long long>(this->current_token->value),
                this->current_token->position));
    this->next_token();
    return result;
}

// STRING_EXPR = STRING;
ExprPtr Parser::parse_string_expr()
{
    if (this->current_token != TokenType::STRING)
        return nullptr;
    auto result = std::make_unique<Expression>(
        StringExpr(std::get<std::wstring>(this->current_token->value),
                   this->current_token->position));
    this->next_token();
    return result;
}

// CHAR_EXPR = CHAR;
ExprPtr Parser::parse_char_expr()
{
    if (this->current_token != TokenType::CHAR)
        return nullptr;
    auto result = std::make_unique<Expression>(
        CharExpr(std::get<wchar_t>(this->current_token->value),
                 this->current_token->position));
    this->next_token();
    return result;
}

// BOOL_EXPR = KW_FALSE | KW_TRUE;
ExprPtr Parser::parse_bool_expr()
{
    if (this->current_token == TokenType::KW_TRUE ||
        this->current_token == TokenType::KW_FALSE)
    {
        auto value = this->current_token == TokenType::KW_TRUE;
        auto position = this->current_token->position;
        auto result = std::make_unique<Expression>(BoolExpr(value, position));
        this->next_token();
        return result;
    }
    else
        return nullptr;
}

namespace
{
void join_into_binary_op(std::stack<ExprPtr> &values,
                         std::stack<BinOpData> &ops)
{
    auto op = ops.top().type;
    ops.pop();

    // moving the nodes out of the values stack - the pointers
    // stored in it must release the ownership
    auto rhs = std::move(values.top());
    values.pop();

    auto lhs = std::move(values.top());
    values.pop();

    auto position = get_position(*lhs);
    auto new_expr = std::make_unique<Expression>(
        BinaryExpr(std::move(lhs), std::move(rhs), op, position));
    values.push(std::move(new_expr));
}

} // namespace

std::optional<BinOpData> Parser::parse_binop()
{
    decltype(this->binary_map)::const_iterator iter;
    if (this->current_token.has_value() &&
        (iter = this->binary_map.find(this->current_token->type)) !=
            this->binary_map.end())
    {
        return iter->second;
    }
    return std::nullopt;
}

// BINARY_EXPR = CAST_EXPR, {BINARY_OP, CAST_EXPR};
ExprPtr Parser::parse_binary_expr()
{
    std::stack<ExprPtr> values;
    std::stack<BinOpData> ops;
    if (auto expr = this->parse_cast_expr())
        values.push(std::move(expr));
    else
        return nullptr;
    while (auto binop = this->parse_binop())
    {
        while (!ops.empty() && (binop->precedence < ops.top().precedence ||
                                (binop->precedence == ops.top().precedence &&
                                 !binop->is_right_assoc)))
            join_into_binary_op(values, ops);

        ops.push(*binop);
        this->next_token();

        if (auto expr = this->parse_cast_expr())
            values.push(std::move(expr));

        else
        {
            this->report_error(
                L"no right-hand side found in a binary expression");
            return nullptr;
        }
    }
    while (!ops.empty())
        join_into_binary_op(values, ops);

    auto result = std::move(values.top());
    values.pop();
    return result;
}

// CAST_EXPR = UNARY_EXPR , {KW_AS, SIMPLE_TYPE};
ExprPtr Parser::parse_cast_expr()
{
    auto lhs = this->parse_unary_expr();
    if (!lhs)
    {
        return nullptr;
    }
    auto position = get_position(*lhs);
    while (this->current_token == TokenType::KW_AS)
    {
        this->next_token();
        if (auto type = this->parse_type())
        {
            lhs = std::make_unique<Expression>(
                CastExpr(std::move(lhs), *type, position));
        }
        else
        {
            this->report_error(L"no type name given in a cast expression");
        }
        position = get_position(*lhs);
    }
    return lhs;
}

std::optional<std::tuple<UnaryOpEnum, Position>> Parser::parse_unop()
{
    if (this->current_token == TokenType::AMPERSAND)
    {
        auto position = this->current_token->position;
        this->next_token();
        if (this->current_token == TokenType::KW_MUT)
        {
            this->next_token();
            return std::make_tuple(UnaryOpEnum::MUT_REF, position);
        }
        return std::make_tuple(UnaryOpEnum::REF, position);
    }
    decltype(this->unary_map)::const_iterator op_iter;
    if (this->current_token.has_value() &&
        (op_iter = this->unary_map.find(this->current_token->type)) !=
            this->unary_map.end())
    {
        auto position = this->current_token->position;
        this->next_token();
        return std::make_tuple(op_iter->second, position);
    }
    return std::nullopt;
}

// UNARY_EXPR = {UNARY_OP}, (INDEX_LAMBDA_OR_CALL);
ExprPtr Parser::parse_unary_expr()
{
    std::stack<UnaryOpEnum> ops;
    std::stack<Position> positions;
    while (auto op_and_pos = this->parse_unop())
    {
        auto [op, pos] = *op_and_pos;
        ops.push(op);
        positions.push(pos);
    }
    auto inner = this->parse_index_expr();
    if (inner)
    {
        while (!ops.empty())
        {
            inner = std::make_unique<Expression>(
                UnaryExpr(std::move(inner), ops.top(), positions.top()));
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

// INDEX_LAMBDA_OR_CALL = VARIABLE_EXPR, {INDEX_PART | CALL_PART};
ExprPtr Parser::parse_index_expr()
{
    auto result = this->parse_factor();
    if (!result)
        return nullptr;

    if (auto new_result = this->parse_index(std::move(result)))
    {
        result = std::move(new_result);
    }

    return result;
}

// CALL_PART = L_PAREN, [ARGS], R_PAREN;
ExprPtr Parser::parse_call(const std::wstring &name, const Position &position)
{
    if (this->current_token != TokenType::L_PAREN)
        return nullptr;
    this->next_token();
    auto args = this->parse_args();
    if (!this->assert_current_and_eat(
            TokenType::R_PAREN,
            L"expected right parenthesis in call expression"))
        return nullptr;
    return std::make_unique<Expression>(
        CallExpr(name, std::move(args), position));
}

// ARGS = BINARY_EXPR, {COMMA, BINARY_EXPR};
std::vector<ExprPtr> Parser::parse_args()
{
    ExprPtr arg;
    std::vector<ExprPtr> args;
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

// INDEX_PART = L_SQ_BRACKET, BINARY_EXPR, R_SQ_BRACKET;
ExprPtr Parser::parse_index(ExprPtr &&expr)
{
    if (this->current_token != TokenType::L_SQ_BRACKET)
        return nullptr;
    this->next_token();
    auto param = this->parse_binary_expr();
    if (!param)
    {
        this->report_error(L"no index value expression found in the index "
                           L"expression's square brackets");
        return nullptr;
    }
    if (!this->assert_current_and_eat(
            TokenType::R_SQ_BRACKET,
            L"expected right square bracket in an index expression"))
        return nullptr;

    auto position = get_position(*expr);
    return std::make_unique<Expression>(
        IndexExpr(std::move(expr), std::move(param), position));
}

// FACTOR = PAREN_EXPR | U32_EXPR | F64_EXPR | STRING_EXPR | CHAR_EXPR |
// BOOL_EXPR;
ExprPtr Parser::parse_factor()
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
    else if (auto result = this->parse_variable_or_call())
        return result;
    else if (auto result = this->parse_paren_expr())
        return result;
    return nullptr;
}

// PAREN_EXPR = L_PAREN, BINARY_EXPR, R_PAREN;
ExprPtr Parser::parse_paren_expr()
{
    if (this->current_token != TokenType::L_PAREN)
        return nullptr;
    auto position = this->current_token->position;
    this->next_token();

    auto expr = this->parse_binary_expr();
    if (!expr)
    {
        this->report_error(L"no expression found after a left parenthesis");
        return nullptr;
    }
    if (!this->assert_current_and_eat(
            TokenType::R_PAREN,
            L"expected a right bracket in a parenthesis expression"))
        return nullptr;

    set_expr_position(*expr, position);
    return expr;
}

LexerPtr Parser::attach_lexer(LexerPtr &lexer) noexcept
{
    auto temp = std::move(this->lexer);
    this->lexer = std::move(lexer);
    this->next_token();
    return temp;
}

LexerPtr Parser::detach_lexer() noexcept
{
    auto temp = std::move(this->lexer);
    this->lexer = nullptr;
    return temp;
}

bool Parser::is_lexer_attached() const noexcept
{
    return this->lexer != nullptr;
}