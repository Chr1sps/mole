#ifndef __AST_HPP__
#define __AST_HPP__
#include "overloaded.hpp"
#include "position.hpp"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// ==========================================================
// =======================DECLARATIONS=======================
// ==========================================================

struct AstNode
{

    Position position;

    virtual ~AstNode()
    {
    }

  protected:
    constexpr AstNode(const Position &position) noexcept : position(position)
    {
    }
};

using AstNodePtr = std::unique_ptr<AstNode>;

// =================
// ===== TYPES =====
// =================

enum class TypeEnum
{
    BOOL,

    I32,

    U32,

    F64,

    CHAR,
    STR,
};

enum class RefSpecifier
{
    NON_REF,
    REF,
    MUT_REF
};

struct SimpleType;
struct FunctionType;
using Type = std::variant<SimpleType, FunctionType>;
using TypePtr = std::unique_ptr<Type>;

// This function is (unfortunatelly) needed to transfer around TypePtrs, since
// certain std::variants cannot be used in std::optionals.
constexpr TypePtr clone_type_ptr(const TypePtr &type) noexcept;

struct SimpleType
{
    TypeEnum type;
    RefSpecifier ref_spec;

    constexpr SimpleType(const TypeEnum &type,
                         const RefSpecifier &ref_spec) noexcept;

    constexpr SimpleType(const SimpleType &) noexcept = default;
    constexpr SimpleType(SimpleType &&) noexcept = default;
    constexpr SimpleType &operator=(const SimpleType &) noexcept = default;
};

struct FunctionType
{
    std::vector<TypePtr> arg_types;
    TypePtr return_type;
    bool is_const;

    constexpr FunctionType(std::vector<TypePtr> arg_types, TypePtr return_type,
                           const bool &is_const) noexcept;

    constexpr FunctionType(const FunctionType &other) noexcept;

    constexpr FunctionType(FunctionType &&) noexcept = default;
    constexpr FunctionType &operator=(const FunctionType &other) noexcept;
};

// =======================
// ===== EXPRESSIONS =====
// =======================

struct VariableExpr;
struct BinaryExpr;
struct UnaryExpr;
struct CallExpr;
struct LambdaCallExpr;
struct IndexExpr;
struct CastExpr;
struct U32Expr;
struct F64Expr;
struct BoolExpr;
struct StringExpr;
struct CharExpr;

using Expression = std::variant<VariableExpr, BinaryExpr, UnaryExpr, CallExpr,
                                LambdaCallExpr, IndexExpr, CastExpr, U32Expr,
                                F64Expr, BoolExpr, StringExpr, CharExpr>;
using ExprPtr = std::unique_ptr<Expression>;

struct VariableExpr : public AstNode
{
    std::wstring name;

    constexpr VariableExpr(const std::wstring &name,
                           const Position &position) noexcept;
};

enum class BinOpEnum
{
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    EXP,

    EQ,
    NEQ,
    GT,
    GE,
    LT,
    LE,

    AND,
    OR,

    BIT_AND,
    BIT_OR,
    BIT_XOR,

    SHL,
    SHR,
};

enum class UnaryOpEnum
{
    MINUS,

    BIT_NEG,

    NEG,

    REF,
    MUT_REF,

    DEREF
};

struct BinaryExpr : public AstNode
{
    ExprPtr lhs, rhs;
    BinOpEnum op;

    constexpr BinaryExpr(ExprPtr lhs, ExprPtr rhs, const BinOpEnum &op,
                         const Position &position) noexcept;
};

struct UnaryExpr : public AstNode
{
    ExprPtr expr;
    UnaryOpEnum op;

    constexpr UnaryExpr(ExprPtr expr, const UnaryOpEnum &op,
                        const Position &position) noexcept;
};

struct CallExpr : public AstNode
{
    ExprPtr callable;
    std::vector<ExprPtr> args;

    constexpr CallExpr(ExprPtr callable, std::vector<ExprPtr> args,
                       const Position &position) noexcept;
};

struct LambdaCallExpr : public AstNode
{
    ExprPtr callable;
    std::vector<ExprPtr> args;

    constexpr LambdaCallExpr(ExprPtr callable, std::vector<ExprPtr> args,
                             const Position &position) noexcept;
};

struct IndexExpr : public AstNode
{
    ExprPtr expr, index_value;

    constexpr IndexExpr(ExprPtr expr, ExprPtr index_value,
                        const Position &position) noexcept;
};

struct CastExpr : public AstNode
{
    ExprPtr expr;
    TypePtr type;

    constexpr CastExpr(ExprPtr expr, TypePtr type,
                       const Position &position) noexcept;
};

struct U32Expr : public AstNode
{
    static const std::shared_ptr<SimpleType> type;
    unsigned long long value;

    constexpr U32Expr(const unsigned long long &value,
                      const Position &position) noexcept;
};

struct F64Expr : public AstNode
{
    static const std::shared_ptr<SimpleType> type;
    double value;

    constexpr F64Expr(const double &value, const Position &position) noexcept;
};

struct StringExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    std::wstring value;

    constexpr StringExpr(const std::wstring &value,
                         const Position &position) noexcept;
};

struct CharExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    wchar_t value;

    constexpr CharExpr(const wchar_t &value,
                       const Position &position) noexcept;
};

struct BoolExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    bool value;

    constexpr BoolExpr(const bool &value, const Position &position) noexcept;
};

constexpr Position get_expr_position(const Expression &expr);
constexpr void set_expr_position(Expression &expr,
                                 const Position &position) noexcept;

// ======================
// ===== STATEMENTS =====
// ======================

struct Block;
struct IfStmt;
struct WhileStmt;
struct MatchStmt;
struct BreakStmt;
struct ContinueStmt;
struct ReturnStmt;
struct FuncDefStmt;
struct AssignStmt;
struct ExprStmt;
struct VarDeclStmt;
struct ExternStmt;
using Statement = std::variant<Block, ReturnStmt, ContinueStmt, BreakStmt,
                               VarDeclStmt, AssignStmt, ExprStmt, WhileStmt,
                               IfStmt, MatchStmt, FuncDefStmt, ExternStmt>;
using StmtPtr = std::unique_ptr<Statement>;
using BlockPtr = std::unique_ptr<Block>;

struct Block : public AstNode
{
    std::vector<StmtPtr> statements;

    constexpr Block(std::vector<StmtPtr> statements,
                    const Position &position) noexcept;
};

struct ReturnStmt : public AstNode
{
    ExprPtr expr;

    constexpr ReturnStmt(const Position &position) noexcept;
    constexpr ReturnStmt(ExprPtr expr, const Position &position) noexcept;
};

struct ContinueStmt : public AstNode
{
    constexpr ContinueStmt(const Position &position) noexcept;
};

struct BreakStmt : public AstNode
{
    constexpr BreakStmt(const Position &position) noexcept;
};

enum class AssignType
{
    NORMAL,

    PLUS,
    MINUS,
    MUL,
    DIV,

    MOD,
    EXP,

    BIT_AND,
    BIT_OR,
    BIT_XOR,

    SHL,
    SHR,
};

struct AssignStmt : public AstNode
{
    ExprPtr lhs, rhs;
    AssignType type;

    constexpr AssignStmt(ExprPtr lhs, const AssignType &type, ExprPtr rhs,
                         const Position &position) noexcept;
};

struct ExprStmt : public AstNode
{
    ExprPtr expr;

    constexpr ExprStmt(ExprPtr expr, const Position &position) noexcept;
};

struct WhileStmt : public AstNode
{
    ExprPtr condition_expr;
    StmtPtr statement;

    constexpr WhileStmt(ExprPtr condition_expr, StmtPtr statement,
                        const Position &position) noexcept;
};

struct IfStmt : public AstNode
{
    ExprPtr condition_expr;
    StmtPtr then_block;
    StmtPtr else_block;

    constexpr IfStmt(ExprPtr condition_expr, StmtPtr then_block,
                     StmtPtr else_block, const Position &position) noexcept;
};

struct LiteralArm;
struct GuardArm;
struct ElseArm;

using MatchArm = std::variant<LiteralArm, GuardArm, ElseArm>;
using MatchArmPtr = std::unique_ptr<MatchArm>;

struct MatchStmt : public AstNode
{
    ExprPtr matched_expr;
    std::vector<MatchArmPtr> match_arms;

    constexpr MatchStmt(ExprPtr matched_expr,
                        std::vector<MatchArmPtr> match_arms,
                        const Position &position) noexcept;
};

struct VarDeclStmt : public AstNode
{
    std::wstring name;
    TypePtr type;
    ExprPtr initial_value;
    bool is_mut;

    constexpr VarDeclStmt(const std::wstring &name, TypePtr type,
                          ExprPtr value, const bool &is_mut,
                          const Position &position) noexcept;
};

struct Parameter;

using ParamPtr = std::unique_ptr<Parameter>;

struct FuncDefStmt : public AstNode
{
    std::wstring name;
    std::vector<ParamPtr> params;
    TypePtr return_type;
    BlockPtr block;
    bool is_const;

    constexpr FuncDefStmt(const std::wstring &name,
                          std::vector<ParamPtr> params, TypePtr return_type,
                          BlockPtr block, const bool &is_const,
                          const Position &position) noexcept;

    constexpr std::unique_ptr<FunctionType> get_type() const noexcept;
};

struct ExternStmt : public AstNode
{
    std::wstring name;
    std::vector<ParamPtr> params;
    TypePtr return_type;

    constexpr ExternStmt(const std::wstring &name,
                         std::vector<ParamPtr> params, TypePtr return_type,
                         const Position &position) noexcept;

    constexpr std::unique_ptr<FunctionType> get_type() const noexcept;
};

// ======================
// ===== MATCH ARMS =====
// ======================

struct MatchArmBase : public AstNode
{
    StmtPtr block;

    constexpr MatchArmBase(StmtPtr block, const Position &position) noexcept;
};

struct ElseArm : public MatchArmBase
{
    constexpr ElseArm(StmtPtr block, const Position &position) noexcept;
};

struct GuardArm : public MatchArmBase
{
    ExprPtr condition_expr;

    constexpr GuardArm(ExprPtr condition_expr, StmtPtr block,
                       const Position &position) noexcept;
};

struct LiteralArm : public MatchArmBase

{
    std::vector<ExprPtr> literals;

    constexpr LiteralArm(std::vector<ExprPtr> literals, StmtPtr block,
                         const Position &position) noexcept;
};

// ===================
// ===== PROGRAM =====
// ===================
struct Program : public AstNode
{
    std::vector<std::unique_ptr<VarDeclStmt>> globals;
    std::vector<std::unique_ptr<FuncDefStmt>> functions;
    std::vector<std::unique_ptr<ExternStmt>> externs;

    constexpr Program(
        std::vector<std::unique_ptr<VarDeclStmt>> globals,
        std::vector<std::unique_ptr<FuncDefStmt>> functions,
        std::vector<std::unique_ptr<ExternStmt>> externs) noexcept;
};

using ProgramPtr = std::unique_ptr<Program>;

// =====================
// ===== PARAMETER =====
// =====================

struct Parameter : public AstNode
{
    std::wstring name;
    TypePtr type;

    constexpr Parameter(const std::wstring &name, TypePtr type,
                        const Position &position) noexcept;
};

constexpr bool operator==(const Type &, const Type &) noexcept;
constexpr bool operator==(const Expression &, const Expression &) noexcept;
constexpr bool operator==(const Block &, const Block &) noexcept;
constexpr bool operator==(const VarDeclStmt &, const VarDeclStmt &) noexcept;
constexpr bool operator==(const FuncDefStmt &, const FuncDefStmt &) noexcept;
constexpr bool operator==(const ExternStmt &, const ExternStmt &) noexcept;
constexpr bool operator==(const Statement &, const Statement &) noexcept;
constexpr bool operator==(const MatchArm &, const MatchArm &) noexcept;
constexpr bool operator==(const Parameter &, const Parameter &) noexcept;
constexpr bool operator==(const Program &, const Program &) noexcept;

// =========================================================
// =======================DEFINITIONS=======================
// =========================================================

// =================
// ===== TYPES =====
// =================

constexpr TypePtr clone_type_ptr(const TypePtr &type) noexcept
{
    if (type)
        return std::make_unique<Type>(*type);
    return nullptr;
}

constexpr SimpleType::SimpleType(const TypeEnum &type,
                                 const RefSpecifier &ref_spec) noexcept
    : type(type), ref_spec(ref_spec)
{
}

constexpr FunctionType::FunctionType(std::vector<TypePtr> arg_types,
                                     TypePtr return_type,
                                     const bool &is_const) noexcept
    : arg_types(std::move(arg_types)), return_type(std::move(return_type)),
      is_const(is_const)
{
}

constexpr FunctionType::FunctionType(const FunctionType &other) noexcept
    : arg_types(), return_type(clone_type_ptr(other.return_type)),
      is_const(other.is_const)
{
    std::transform(other.arg_types.cbegin(), other.arg_types.cend(),
                   std::back_inserter(this->arg_types),
                   [](const TypePtr &arg) { return clone_type_ptr(arg); });
}

constexpr FunctionType &FunctionType::operator=(
    const FunctionType &other) noexcept
{
    std::transform(other.arg_types.cbegin(), other.arg_types.cend(),
                   std::back_inserter(this->arg_types),
                   [](const TypePtr &arg) { return clone_type_ptr(arg); });
    this->return_type = clone_type_ptr(other.return_type);
    this->is_const = other.is_const;
    return *this;
}

// =======================
// ===== EXPRESSIONS =====
// =======================

constexpr VariableExpr::VariableExpr(const std::wstring &name,
                                     const Position &position) noexcept
    : AstNode(position), name(name)
{
}

constexpr BinaryExpr::BinaryExpr(ExprPtr lhs, ExprPtr rhs, const BinOpEnum &op,
                                 const Position &position) noexcept
    : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
{
}

constexpr UnaryExpr::UnaryExpr(ExprPtr expr, const UnaryOpEnum &op,
                               const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)), op(op)
{
}

constexpr CallExpr::CallExpr(ExprPtr callable, std::vector<ExprPtr> args,
                             const Position &position) noexcept
    : AstNode(position), callable(std::move(callable)), args(std::move(args))
{
}

constexpr LambdaCallExpr::LambdaCallExpr(ExprPtr callable,
                                         std::vector<ExprPtr> args,
                                         const Position &position) noexcept
    : AstNode(position), callable(std::move(callable)), args(std::move(args))
{
}

constexpr IndexExpr::IndexExpr(ExprPtr expr, ExprPtr index_value,
                               const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)),
      index_value(std::move(index_value))
{
}

constexpr CastExpr::CastExpr(ExprPtr expr, TypePtr type,
                             const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)), type(std::move(type))
{
}

inline const std::shared_ptr<SimpleType> U32Expr::type =
    std::make_shared<SimpleType>(TypeEnum::U32, RefSpecifier::NON_REF);

constexpr U32Expr::U32Expr(const unsigned long long &value,
                           const Position &position) noexcept
    : AstNode(position), value(value)
{
}

inline const std::shared_ptr<SimpleType> F64Expr::type =
    std::make_shared<SimpleType>(TypeEnum::F64, RefSpecifier::NON_REF);

constexpr F64Expr::F64Expr(const double &value,
                           const Position &position) noexcept
    : AstNode(position), value(value)
{
}

constexpr StringExpr::StringExpr(const std::wstring &value,
                                 const Position &position) noexcept
    : AstNode(position), value(value)
{
}

constexpr CharExpr::CharExpr(const wchar_t &value,
                             const Position &position) noexcept
    : AstNode(position), value(value)
{
}

constexpr BoolExpr::BoolExpr(const bool &value,
                             const Position &position) noexcept
    : AstNode(position), value(value)
{
}

constexpr Position get_expr_position(const Expression &expr)
{
    return std::visit(
        [](const AstNode &node) -> Position { return node.position; }, expr);
}

constexpr void set_expr_position(Expression &expr,
                                 const Position &position) noexcept
{
    std::visit([&position](AstNode &node) { node.position = position; }, expr);
}

// ======================
// ===== STATEMENTS =====
// ======================

constexpr ReturnStmt::ReturnStmt(const Position &position) noexcept
    : AstNode(position), expr()
{
}

constexpr ReturnStmt::ReturnStmt(ExprPtr expr,
                                 const Position &position) noexcept
    : AstNode(position), expr(std::move(expr))
{
}

constexpr ContinueStmt::ContinueStmt(const Position &position) noexcept
    : AstNode(position)
{
}

constexpr BreakStmt::BreakStmt(const Position &position) noexcept
    : AstNode(position)
{
}

constexpr AssignStmt::AssignStmt(ExprPtr lhs, const AssignType &type,
                                 ExprPtr rhs,
                                 const Position &position) noexcept
    : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), type(type)
{
}

constexpr ExprStmt::ExprStmt(ExprPtr expr, const Position &position) noexcept
    : AstNode(position), expr(std::move(expr))
{
}

constexpr WhileStmt::WhileStmt(ExprPtr condition_expr, StmtPtr statement,
                               const Position &position) noexcept
    : AstNode(position), condition_expr(std::move(condition_expr)),
      statement(std::move(statement))
{
}

constexpr IfStmt::IfStmt(ExprPtr condition_expr, StmtPtr then_block,
                         StmtPtr else_block, const Position &position) noexcept
    : AstNode(position), condition_expr(std::move(condition_expr)),
      then_block(std::move(then_block)), else_block(std::move(else_block))
{
}

constexpr MatchStmt::MatchStmt(ExprPtr matched_expr,
                               std::vector<MatchArmPtr> match_arms,
                               const Position &position) noexcept
    : AstNode(position), matched_expr(std::move(matched_expr)),
      match_arms(std::move(match_arms))
{
}

constexpr Block::Block(std::vector<StmtPtr> statements,
                       const Position &position) noexcept
    : AstNode(position), statements(std::move(statements))
{
}

constexpr VarDeclStmt::VarDeclStmt(const std::wstring &name, TypePtr type,
                                   ExprPtr value, const bool &is_mut,
                                   const Position &position) noexcept
    : AstNode(position), name(name), type(std::move(type)),
      initial_value(std::move(value)), is_mut(is_mut)
{
}

constexpr FuncDefStmt::FuncDefStmt(const std::wstring &name,
                                   std::vector<ParamPtr> params,
                                   TypePtr return_type, BlockPtr block,
                                   const bool &is_const,
                                   const Position &position) noexcept
    : AstNode(position), name(name), params(std::move(params)),
      return_type(std::move(return_type)), block(std::move(block)),
      is_const(is_const)
{
}

constexpr std::unique_ptr<FunctionType> FuncDefStmt::get_type() const noexcept
{
    std::vector<TypePtr> param_types;
    for (auto &param_ptr : this->params)
        param_types.push_back(clone_type_ptr(param_ptr->type));
    auto cloned_type = clone_type_ptr(this->return_type);
    return std::make_unique<FunctionType>(
        std::move(param_types), std::move(cloned_type), this->is_const);
}

constexpr ExternStmt::ExternStmt(const std::wstring &name,
                                 std::vector<ParamPtr> params,
                                 TypePtr return_type,
                                 const Position &position) noexcept
    : AstNode(position), name(name), params(std::move(params)),
      return_type(std::move(return_type))
{
}

constexpr std::unique_ptr<FunctionType> ExternStmt::get_type() const noexcept
{
    std::vector<TypePtr> param_types;
    for (auto &param_ptr : this->params)
        param_types.push_back(clone_type_ptr(param_ptr->type));
    auto cloned_type = clone_type_ptr(this->return_type);
    return std::make_unique<FunctionType>(std::move(param_types),
                                          std::move(cloned_type), false);
}

// ======================
// ===== MATCH ARMS =====
// ======================

constexpr MatchArmBase::MatchArmBase(StmtPtr block,
                                     const Position &position) noexcept
    : AstNode(position), block(std::move(block))
{
}

constexpr GuardArm::GuardArm(ExprPtr condition_expr, StmtPtr block,
                             const Position &position) noexcept
    : MatchArmBase(std::move(block), position),
      condition_expr(std::move(condition_expr))
{
}

constexpr LiteralArm::LiteralArm(std::vector<ExprPtr> literals, StmtPtr block,
                                 const Position &position) noexcept
    : MatchArmBase(std::move(block), position), literals(std::move(literals))
{
}

constexpr ElseArm::ElseArm(StmtPtr block, const Position &position) noexcept
    : MatchArmBase(std::move(block), position)
{
}

// =====================
// ===== PARAMETER =====
// =====================

constexpr Parameter::Parameter(const std::wstring &name, TypePtr type,
                               const Position &position) noexcept
    : AstNode(position), name(name), type(std::move(type))
{
}

// ===================
// ===== PROGRAM =====
// ===================

constexpr Program::Program(
    std::vector<std::unique_ptr<VarDeclStmt>> globals,
    std::vector<std::unique_ptr<FuncDefStmt>> functions,
    std::vector<std::unique_ptr<ExternStmt>> externs) noexcept
    : AstNode(Position(1, 1)), globals(std::move(globals)),
      functions(std::move(functions)), externs(std::move(externs))
{
}

namespace
{
template <typename T>
constexpr bool equal_or_null(
    const std::unique_ptr<T> &first,
    const std::unique_ptr<T> &other) noexcept(noexcept(std::declval<T>() ==
                                                       std::declval<T>()))
{
    return (first == nullptr && other == nullptr) ||
           (first && other && *first == *other);
}

template <typename T>
constexpr bool compare_ptr_vectors(
    const std::vector<std::unique_ptr<T>> &first,
    const std::vector<std::unique_ptr<T>>
        &second) noexcept(noexcept(std::declval<T>() == std::declval<T>()))

{
    return std::equal(
        first.cbegin(), first.cend(), second.cbegin(), second.cend(),
        [](const std::unique_ptr<T> &a, const std::unique_ptr<T> &b) {
            return a && b && *a == *b;
        });
}

} // namespace

constexpr bool operator==(const Type &first, const Type &other) noexcept
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

constexpr bool operator==(const Expression &first,
                          const Expression &other) noexcept
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
                       std::equal(first.args.begin(), first.args.end(),
                                  other.args.begin(), other.args.end(),
                                  [](const ExprPtr &a, const ExprPtr &b) {
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

constexpr bool operator==(const Block &first, const Block &other) noexcept
{
    return compare_ptr_vectors(first.statements, other.statements) &&
           first.position == other.position;
}

constexpr bool operator==(const VarDeclStmt &first,
                          const VarDeclStmt &other) noexcept
{
    return first.name == other.name && equal_or_null(first.type, other.type) &&
           equal_or_null(first.initial_value, other.initial_value) &&
           first.is_mut == other.is_mut && first.position == other.position;
}

constexpr bool operator==(const FuncDefStmt &first,
                          const FuncDefStmt &other) noexcept
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

constexpr bool operator==(const ExternStmt &first,
                          const ExternStmt &other) noexcept
{
    return first.name == other.name &&
           compare_ptr_vectors(first.params, other.params) &&
           equal_or_null(first.return_type, other.return_type) &&
           first.position == other.position;
}

constexpr bool operator==(const Statement &first,
                          const Statement &second) noexcept
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

constexpr bool operator==(const MatchArm &first,
                          const MatchArm &other) noexcept
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

constexpr bool operator==(const Program &first, const Program &other) noexcept
{
    return compare_ptr_vectors(first.externs, other.externs) &&
           compare_ptr_vectors(first.functions, other.functions) &&
           compare_ptr_vectors(first.globals, other.globals) &&
           first.position == other.position; // not needed, but whatever
}

constexpr bool operator==(const Parameter &first,
                          const Parameter &other) noexcept
{
    return first.name == other.name && *first.type == *other.type &&
           first.position == other.position;
}
#endif