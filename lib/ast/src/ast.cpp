#include "ast.hpp"
#include "overloaded.hpp"
#include <utility>
const std::shared_ptr<SimpleType> U32Expr::type =
    std::make_shared<SimpleType>(TypeEnum::I32, RefSpecifier::NON_REF);
const std::shared_ptr<SimpleType> F64Expr::type =
    std::make_shared<SimpleType>(TypeEnum::F64, RefSpecifier::NON_REF);

namespace
{
template <typename T>
bool equal_or_null(const std::unique_ptr<T> &first,
                   const std::unique_ptr<T> &other)
// noexcept(noexcept(std::declval<T>() ==
//                                                    std::declval<T>()))
{
    return (first == nullptr && other == nullptr) ||
           (first && other && *first == *other);
}

template <typename T>
bool compare_ptr_vectors(const std::vector<std::unique_ptr<T>> &first,
                         const std::vector<std::unique_ptr<T>> &second)
//  noexcept(noexcept(std::declval<T>() ==
//                             std::declval<T>()))

{
    return std::equal(
        first.cbegin(), first.cend(), second.cbegin(), second.cend(),
        [](const std::unique_ptr<T> &a, const std::unique_ptr<T> &b) {
            return a && b && *a == *b;
        });
}

TypePtr clone_type_ptr(const TypePtr &type) noexcept
{
    return std::make_unique<Type>(*type);
}
} // namespace

// =================
// ===== TYPES =====
// =================

SimpleType::SimpleType(const TypeEnum &type,
                       const RefSpecifier &ref_spec) noexcept
    : type(type), ref_spec(ref_spec)
{
}

FunctionType::FunctionType(std::vector<TypePtr> arg_types, TypePtr return_type,
                           const bool &is_const) noexcept
    : arg_types(std::move(arg_types)), return_type(std::move(return_type)),
      is_const(is_const)
{
}

FunctionType::FunctionType(const FunctionType &other) noexcept
    : arg_types(), return_type(clone_type_ptr(other.return_type)),
      is_const(other.is_const)
{
    for (const auto &arg : other.arg_types)
    {
        this->arg_types.push_back(clone_type_ptr(arg));
    }
}

bool operator==(const Type &first, const Type &other) noexcept
{
    return std::visit(
        overloaded{
            [](const SimpleType &first, const SimpleType &other) -> bool {
                return first.ref_spec == other.ref_spec &&
                       first.type == other.type;
            },
            [](const FunctionType &first, const FunctionType &other) -> bool {
                return compare_ptr_vectors(first.arg_types, other.arg_types) &&
                       equal_or_null(first.return_type, other.return_type) &&
                       first.is_const == other.is_const;
            },
            [](const auto &, const auto &) -> bool { return false; }},
        first, other);
}

// =======================
// ===== EXPRESSIONS =====
// =======================

VariableExpr::VariableExpr(const std::wstring &name,
                           const Position &position) noexcept
    : AstNode(position), name(name)
{
}

BinaryExpr::BinaryExpr(ExprNodePtr lhs, ExprNodePtr rhs, const BinOpEnum &op,
                       const Position &position) noexcept
    : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
{
}

UnaryExpr::UnaryExpr(ExprNodePtr expr, const UnaryOpEnum &op,
                     const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)), op(op)
{
}

CallExpr::CallExpr(ExprNodePtr callable, std::vector<ExprNodePtr> args,
                   const Position &position) noexcept
    : AstNode(position), callable(std::move(callable)), args(std::move(args))
{
}

LambdaCallExpr::LambdaCallExpr(ExprNodePtr callable,
                               std::vector<ExprNodePtr> args,
                               const Position &position) noexcept
    : AstNode(position), callable(std::move(callable)), args(std::move(args))
{
}

IndexExpr::IndexExpr(ExprNodePtr expr, ExprNodePtr index_value,
                     const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)),
      index_value(std::move(index_value))
{
}

CastExpr::CastExpr(ExprNodePtr expr, TypePtr type,
                   const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)), type(std::move(type))
{
}

U32Expr::U32Expr(const unsigned long long &value,
                 const Position &position) noexcept
    : AstNode(position), value(value)
{
}

F64Expr::F64Expr(const double &value, const Position &position) noexcept
    : AstNode(position), value(value)
{
}

StringExpr::StringExpr(const std::wstring &value,
                       const Position &position) noexcept
    : AstNode(position), value(value)
{
}

CharExpr::CharExpr(const wchar_t &value, const Position &position) noexcept
    : AstNode(position), value(value)
{
}

BoolExpr::BoolExpr(const bool &value, const Position &position) noexcept
    : AstNode(position), value(value)
{
}

bool operator==(const ExprNode &first, const ExprNode &other) noexcept
{
    return std::visit(
        overloaded{
            [](const VariableExpr &first, const VariableExpr &other) -> bool {
                return first.name == other.name &&
                       first.position == other.position;
            },
            [](const U32Expr &first, const U32Expr &other) -> bool {
                return first.value == other.value &&
                       first.position == other.position;
            },
            [](const F64Expr &first, const F64Expr &other) -> bool {
                return first.value == other.value &&
                       first.position == other.position;
            },
            [](const StringExpr &first, const StringExpr &other) -> bool {
                return first.value == other.value &&
                       first.position == other.position;
            },
            [](const CharExpr &first, const CharExpr &other) -> bool {
                return first.value == other.value &&
                       first.position == other.position;
            },
            [](const BoolExpr &first, const BoolExpr &other) -> bool {
                return first.value == other.value &&
                       first.position == other.position;
            },
            [](const UnaryExpr &first, const UnaryExpr &other) -> bool {
                return *first.expr == *other.expr && first.op == other.op &&
                       first.position == other.position;
            },
            [](const BinaryExpr &first, const BinaryExpr &other) -> bool {
                return *first.lhs == *other.lhs && *first.rhs == *other.rhs &&
                       first.op == other.op &&
                       first.position == other.position;
            },
            [](const CallExpr &first, const CallExpr &other) -> bool {
                return *first.callable == *other.callable &&
                       compare_ptr_vectors(first.args, other.args) &&
                       first.position == other.position;
            },
            [](const LambdaCallExpr &first,
               const LambdaCallExpr &other) -> bool {
                return *first.callable == *other.callable &&
                       std::equal(
                           first.args.begin(), first.args.end(),
                           other.args.begin(), other.args.end(),
                           [](const ExprNodePtr &a, const ExprNodePtr &b) {
                               return equal_or_null(a, b);
                           }) &&
                       first.position == other.position;
            },
            [](const IndexExpr &first, const IndexExpr &other) -> bool {
                return *first.expr == *other.expr &&
                       *first.index_value == *other.index_value &&
                       first.position == other.position;
            },
            [](const CastExpr &first, const CastExpr &other) -> bool {
                return *first.expr == *other.expr &&
                       *first.type == *other.type &&
                       first.position == other.position;
            },
            [](const auto &, const auto &) -> bool { return false; }},
        first, other);
}

Position get_expr_position(const ExprNode &expr)
{
    return std::visit(
        [](const AstNode &node) -> Position { return node.position; }, expr);
}

void set_expr_position(ExprNode &expr, const Position &position) noexcept
{
    std::visit([&position](AstNode &node) { node.position = position; }, expr);
}

// ======================
// ===== STATEMENTS =====
// ======================

Block::Block(std::vector<StmtPtr> statements,
             const Position &position) noexcept
    : AstNode(position), statements(std::move(statements))
{
}

bool operator==(const Block &first, const Block &other) noexcept
{
    return compare_ptr_vectors(first.statements, other.statements) &&
           first.position == other.position;
}

ReturnStmt::ReturnStmt(const Position &position) noexcept
    : AstNode(position), expr()
{
}

ReturnStmt::ReturnStmt(ExprNodePtr expr, const Position &position) noexcept
    : AstNode(position), expr(std::move(expr))
{
}

ContinueStmt::ContinueStmt(const Position &position) noexcept
    : AstNode(position)
{
}

BreakStmt::BreakStmt(const Position &position) noexcept : AstNode(position)
{
}

AssignStmt::AssignStmt(ExprNodePtr lhs, const AssignType &type,
                       ExprNodePtr rhs, const Position &position) noexcept
    : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), type(type)
{
}

ExprStmt::ExprStmt(ExprNodePtr expr, const Position &position) noexcept
    : AstNode(position), expr(std::move(expr))
{
}

WhileStmt::WhileStmt(ExprNodePtr condition_expr, StmtPtr statement,
                     const Position &position) noexcept
    : AstNode(position), condition_expr(std::move(condition_expr)),
      statement(std::move(statement))
{
}

IfStmt::IfStmt(ExprNodePtr condition_expr, StmtPtr then_block,
               StmtPtr else_block, const Position &position) noexcept
    : AstNode(position), condition_expr(std::move(condition_expr)),
      then_block(std::move(then_block)), else_block(std::move(else_block))
{
}

MatchStmt::MatchStmt(ExprNodePtr matched_expr,
                     std::vector<MatchArmPtr> match_arms,
                     const Position &position) noexcept
    : AstNode(position), matched_expr(std::move(matched_expr)),
      match_arms(std::move(match_arms))
{
}

VarDeclStmt::VarDeclStmt(const std::wstring &name, TypePtr type,
                         ExprNodePtr value, const bool &is_mut,
                         const Position &position) noexcept
    : AstNode(position), name(name), type(std::move(type)),
      initial_value(std::move(value)), is_mut(is_mut)
{
}

bool operator==(const VarDeclStmt &first, const VarDeclStmt &other) noexcept
{
    return first.name == other.name && equal_or_null(first.type, other.type) &&
           equal_or_null(first.initial_value, other.initial_value) &&
           first.is_mut == other.is_mut && first.position == other.position;
}

FuncDefStmt::FuncDefStmt(const std::wstring &name,
                         std::vector<ParamPtr> params, TypePtr return_type,
                         BlockPtr block, const bool &is_const,
                         const Position &position) noexcept
    : AstNode(position), name(name), params(std::move(params)),
      return_type(std::move(return_type)), block(std::move(block)),
      is_const(is_const)
{
}

bool operator==(const FuncDefStmt &first, const FuncDefStmt &other) noexcept
{
    // auto equal_blocks = *first.block == *other.block;
    auto equal_blocks = compare_ptr_vectors(first.block->statements,
                                            other.block->statements) &&
                        first.block->position == other.block->position;
    return first.name == other.name &&
           compare_ptr_vectors(first.params, other.params) &&
           equal_or_null(first.return_type, other.return_type) &&
           equal_blocks && first.is_const == other.is_const &&
           first.position == other.position;
}

std::unique_ptr<FunctionType> FuncDefStmt::get_type() const noexcept
{
    std::vector<TypePtr> param_types;
    for (auto &param_ptr : this->params)
        param_types.push_back(clone_type_ptr(param_ptr->type));
    auto cloned_type = clone_type_ptr(this->return_type);
    return std::make_unique<FunctionType>(
        std::move(param_types), std::move(cloned_type), this->is_const);
}

ExternStmt::ExternStmt(const std::wstring &name, std::vector<ParamPtr> params,
                       TypePtr return_type, const Position &position) noexcept
    : AstNode(position), name(name), params(std::move(params)),
      return_type(std::move(return_type))
{
}

std::unique_ptr<FunctionType> ExternStmt::get_type() const noexcept
{
    std::vector<TypePtr> param_types;
    for (auto &param_ptr : this->params)
        param_types.push_back(clone_type_ptr(param_ptr->type));
    auto cloned_type = clone_type_ptr(this->return_type);
    return std::make_unique<FunctionType>(std::move(param_types),
                                          std::move(cloned_type), false);
}

bool operator==(const ExternStmt &first, const ExternStmt &other) noexcept
{
    return first.name == other.name &&
           compare_ptr_vectors(first.params, other.params) &&
           equal_or_null(first.return_type, other.return_type) &&
           first.position == other.position;
}

bool operator==(const Statement &first, const Statement &second) noexcept
{
    return std::visit(
        overloaded{
            [](const IfStmt &first, const IfStmt &other) -> bool {
                return *first.condition_expr == *other.condition_expr &&
                       *first.then_block == *other.then_block &&
                       equal_or_null(first.else_block, other.else_block) &&
                       first.position == other.position;
            },
            [](const WhileStmt &first, const WhileStmt &other) -> bool {
                return *first.condition_expr == *other.condition_expr &&
                       *first.statement == *other.statement &&
                       first.position == other.position;
            },
            [](const MatchStmt &first, const MatchStmt &other) -> bool {
                return *first.matched_expr == *other.matched_expr &&
                       compare_ptr_vectors(first.match_arms,
                                           other.match_arms) &&
                       first.position == other.position;
            },
            [](const ReturnStmt &first, const ReturnStmt &other) -> bool {
                return equal_or_null(first.expr, other.expr) &&
                       first.position == other.position;
            },
            [](const ContinueStmt &first, const ContinueStmt &other) -> bool {
                return first.position == other.position;
            },
            [](const BreakStmt &first, const BreakStmt &other) -> bool {
                return first.position == other.position;
            },
            [](const ExprStmt &first, const ExprStmt &other) -> bool {
                return *first.expr == *other.expr &&
                       first.position == other.position;
            },
            [](const AssignStmt &first, const AssignStmt &other) -> bool {
                return *first.lhs == *other.lhs && first.type == other.type &&
                       *first.rhs == *other.rhs &&
                       first.position == other.position;
            },
            [](const Block &first, const Block &other) -> bool {
                // return first == other;
                return compare_ptr_vectors(first.statements,
                                           other.statements) &&
                       first.position == other.position;
            },
            [](const FuncDefStmt &first, const FuncDefStmt &other) -> bool {
                return first == other;
            },
            [](const VarDeclStmt &first, const VarDeclStmt &other) -> bool {
                return first == other;
            },
            [](const ExternStmt &first, const ExternStmt &other) -> bool {
                return first == other;
            },
            [](const auto &, const auto &) -> bool { return false; }},
        first, second);
}

// ======================
// ===== MATCH ARMS =====
// ======================

MatchArmBase::MatchArmBase(StmtPtr block, const Position &position) noexcept
    : AstNode(position), block(std::move(block))
{
}

GuardArm::GuardArm(ExprNodePtr condition_expr, StmtPtr block,
                   const Position &position) noexcept
    : MatchArmBase(std::move(block), position),
      condition_expr(std::move(condition_expr))
{
}

LiteralArm::LiteralArm(std::vector<ExprNodePtr> literals, StmtPtr block,
                       const Position &position) noexcept
    : MatchArmBase(std::move(block), position), literals(std::move(literals))
{
}

ElseArm::ElseArm(StmtPtr block, const Position &position) noexcept
    : MatchArmBase(std::move(block), position)
{
}

bool operator==(const MatchArm &first, const MatchArm &other) noexcept
{
    return std::visit(
        overloaded{
            [](const LiteralArm &first, const LiteralArm &other) -> bool {
                return compare_ptr_vectors(first.literals, other.literals) &&
                       first.position == other.position;
            },
            [](const GuardArm &first, const GuardArm &other) -> bool {
                return *first.condition_expr == *other.condition_expr &&
                       first.position == other.position;
            },
            [](const ElseArm &first, const ElseArm &other) -> bool {
                return first.position == other.position;
            },
            [](const auto &, const auto &) -> bool { return false; }},
        first, other);
}

// ==================
// ===== OTHERS =====
// ==================

Program::Program(std::vector<std::unique_ptr<VarDeclStmt>> globals,
                 std::vector<std::unique_ptr<FuncDefStmt>> functions,
                 std::vector<std::unique_ptr<ExternStmt>> externs) noexcept
    : AstNode(Position(1, 1)), globals(std::move(globals)),
      functions(std::move(functions)), externs(std::move(externs))
{
}

bool operator==(const Program &first, const Program &other) noexcept
{
    return compare_ptr_vectors(first.externs, other.externs) &&
           compare_ptr_vectors(first.functions, other.functions) &&
           compare_ptr_vectors(first.globals, other.globals) &&
           first.position == other.position; // not needed, but whatever
}

Parameter::Parameter(const std::wstring &name, TypePtr type,
                     const Position &position) noexcept
    : AstNode(position), name(name), type(std::move(type))
{
}

bool operator==(const Parameter &first, const Parameter &other) noexcept
{
    return first.name == other.name && *first.type == *other.type &&
           first.position == other.position;
}