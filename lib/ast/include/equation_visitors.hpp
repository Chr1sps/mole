#ifndef __EQUATION_VISITORS_HPP__
#define __EQUATION_VISITORS_HPP__
#include "ast.hpp"
#include "visitor.hpp"

template <typename T>
bool equal_or_null(const std::unique_ptr<T> &first,
                   const std::unique_ptr<T> &other)
{
    return (first == nullptr && other == nullptr) ||
           (first && other && *first == *other);
}

template <typename T>
bool equal_or_null(const std::shared_ptr<T> &first,
                   const std::shared_ptr<T> &other)
{
    return (first == nullptr && other == nullptr) ||
           (first && other && *first == *other);
}

template <typename T> bool equal_or_null(const T *first, const T *other)
{
    return (first == nullptr && other == nullptr) ||
           (first && other && *first == *other);
}

bool operator==(const MatchArm &first, const MatchArm &second);
bool operator==(const Statement &first, const Statement &second);
bool operator==(const Parameter &first, const Parameter &other);
bool operator==(const Program &first, const Program &other);

template <typename T>
bool compare_ptr_vectors(const std::vector<std::unique_ptr<T>> &first,
                         const std::vector<std::unique_ptr<T>> &second)
{
    return std::equal(
        first.begin(), first.end(), second.begin(), second.end(),
        [](const std::unique_ptr<T> &a, const std::unique_ptr<T> &b) {
            return a && b && *a == *b;
        });
}

bool compare_variant_vectors(const std::vector<ExprNodePtr> &first,
                             const std::vector<ExprNodePtr> &other)
{
    return std::equal(
        first.begin(), first.end(), other.begin(), other.end(),
        [](const ExprNodePtr &a, const ExprNodePtr &b) { return a == b; });
}

bool operator==(const ExprNode &first, const ExprNode &other)
{
    return std::visit(
        overloaded{
            [](const VariableExpr &first, const VariableExpr &other) -> bool {
                return first.name == other.name &&
                       first.position == other.position;
            },
            [](const I32Expr &first, const I32Expr &other) -> bool {
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

bool operator==(const MatchArm &first, const MatchArm &other)
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

bool operator!=(const MatchArm &first, const MatchArm &other)
{
    return !(first == other);
}

bool operator==(const Block &first, const Block &other)
{
    return compare_ptr_vectors(first.statements, other.statements) &&
           first.position == other.position;
}

bool operator==(const FuncDefStmt &first, const FuncDefStmt &other)
{
    return first.name == other.name &&
           compare_ptr_vectors(first.params, other.params) &&
           equal_or_null(first.return_type, other.return_type) &&
           *first.block == *other.block && first.is_const == other.is_const &&
           first.position == other.position;
}

bool operator==(const VarDeclStmt &first, const VarDeclStmt &other)
{
    return first.name == other.name && equal_or_null(first.type, other.type) &&
           equal_or_null(first.initial_value, other.initial_value) &&
           first.is_mut == other.is_mut && first.position == other.position;
}

bool operator==(const ExternStmt &first, const ExternStmt &other)
{
    return first.name == other.name &&
           compare_ptr_vectors(first.params, other.params) &&
           equal_or_null(first.return_type, other.return_type) &&
           first.position == other.position;
}

bool operator==(const Statement &first, const Statement &second)
{
    return std::visit(
        overloaded{
            [](const Block &first, const Block &other) -> bool {
                return first == other;
            },
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

bool operator!=(const Statement &first, const Statement &other)
{
    return !(first == other);
}

bool operator==(const Parameter &first, const Parameter &other)
{
    return first.name == other.name && *first.type == *other.type &&
           first.position == other.position;
}

bool operator!=(const Parameter &first, const Parameter &other)
{
    return !(first == other);
}

bool operator==(const Program &first, const Program &other)
{
    return compare_ptr_vectors(first.externs, other.externs) &&
           compare_ptr_vectors(first.functions, other.functions) &&
           compare_ptr_vectors(first.globals, other.globals) &&
           first.position == other.position; // not needed, but whatever
}

bool operator!=(const Program &first, const Program &other)
{
    return !(first == other);
}
#endif