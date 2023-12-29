#ifndef __AST_HPP__
#define __AST_HPP__
#include "position.hpp"
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
    inline constexpr AstNode(const Position &position) noexcept
        : position(position)
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

namespace
{
constexpr TypePtr clone_type_ptr(const TypePtr &type) noexcept;
} // namespace

struct SimpleType
{
    TypeEnum type;
    RefSpecifier ref_spec;

    constexpr SimpleType(const TypeEnum &type,
                         const RefSpecifier &ref_spec) noexcept;

    constexpr SimpleType(const SimpleType &) noexcept = default;
    constexpr SimpleType(SimpleType &&) noexcept = default;
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
};

bool operator==(const Type &, const Type &) noexcept;

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

using ExprNode = std::variant<VariableExpr, BinaryExpr, UnaryExpr, CallExpr,
                              LambdaCallExpr, IndexExpr, CastExpr, U32Expr,
                              F64Expr, BoolExpr, StringExpr, CharExpr>;
using ExprNodePtr = std::unique_ptr<ExprNode>;

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
    INC,
    DEC,

    NEG,
    BIT_NEG,
    MINUS,
    REF
};

struct BinaryExpr : public AstNode
{
    ExprNodePtr lhs, rhs;
    BinOpEnum op;

    constexpr BinaryExpr(ExprNodePtr lhs, ExprNodePtr rhs, const BinOpEnum &op,
                         const Position &position) noexcept;
};

struct UnaryExpr : public AstNode
{
    ExprNodePtr expr;
    UnaryOpEnum op;

    constexpr UnaryExpr(ExprNodePtr expr, const UnaryOpEnum &op,
                        const Position &position) noexcept;
};

struct CallExpr : public AstNode
{
    ExprNodePtr callable;
    std::vector<ExprNodePtr> args;

    constexpr CallExpr(ExprNodePtr callable, std::vector<ExprNodePtr> args,
                       const Position &position) noexcept;
};

struct LambdaCallExpr : public AstNode
{
    ExprNodePtr callable;
    std::vector<ExprNodePtr> args;

    constexpr LambdaCallExpr(ExprNodePtr callable,
                             std::vector<ExprNodePtr> args,
                             const Position &position) noexcept;
};

struct IndexExpr : public AstNode
{
    ExprNodePtr expr, index_value;

    constexpr IndexExpr(ExprNodePtr expr, ExprNodePtr index_value,
                        const Position &position) noexcept;
};

struct CastExpr : public AstNode
{
    ExprNodePtr expr;
    TypePtr type;

    constexpr CastExpr(ExprNodePtr expr, TypePtr type,
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

bool operator==(const ExprNode &, const ExprNode &) noexcept;

constexpr Position get_expr_position(const ExprNode &expr);
constexpr void set_expr_position(ExprNode &expr,
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

bool operator==(const Block &, const Block &) noexcept;

struct ReturnStmt : public AstNode
{
    ExprNodePtr expr;

    constexpr ReturnStmt(const Position &position) noexcept;
    constexpr ReturnStmt(ExprNodePtr expr, const Position &position) noexcept;
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

    BIT_NEG,
    BIT_AND,
    BIT_OR,
    BIT_XOR,

    SHL,
    SHR,
};

struct AssignStmt : public AstNode
{
    ExprNodePtr lhs, rhs;
    AssignType type;

    constexpr AssignStmt(ExprNodePtr lhs, const AssignType &type,
                         ExprNodePtr rhs, const Position &position) noexcept;
};

struct ExprStmt : public AstNode
{
    ExprNodePtr expr;

    constexpr ExprStmt(ExprNodePtr expr, const Position &position) noexcept;
};

struct WhileStmt : public AstNode
{
    ExprNodePtr condition_expr;
    StmtPtr statement;

    constexpr WhileStmt(ExprNodePtr condition_expr, StmtPtr statement,
                        const Position &position) noexcept;
};

struct IfStmt : public AstNode
{
    ExprNodePtr condition_expr;
    StmtPtr then_block;
    StmtPtr else_block;

    constexpr IfStmt(ExprNodePtr condition_expr, StmtPtr then_block,
                     StmtPtr else_block, const Position &position) noexcept;
};

struct LiteralArm;
struct GuardArm;
struct ElseArm;

using MatchArm = std::variant<LiteralArm, GuardArm, ElseArm>;
using MatchArmPtr = std::unique_ptr<MatchArm>;

struct MatchStmt : public AstNode
{
    ExprNodePtr matched_expr;
    std::vector<MatchArmPtr> match_arms;

    constexpr MatchStmt(ExprNodePtr matched_expr,
                        std::vector<MatchArmPtr> match_arms,
                        const Position &position) noexcept;
};

struct VarDeclStmt : public AstNode
{
    std::wstring name;
    TypePtr type;
    ExprNodePtr initial_value;
    bool is_mut;

    constexpr VarDeclStmt(const std::wstring &name, TypePtr type,
                          ExprNodePtr value, const bool &is_mut,
                          const Position &position) noexcept;
};

bool operator==(const VarDeclStmt &, const VarDeclStmt &) noexcept;

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

bool operator==(const FuncDefStmt &, const FuncDefStmt &) noexcept;

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

bool operator==(const ExternStmt &, const ExternStmt &) noexcept;

bool operator==(const Statement &, const Statement &) noexcept;

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
    ExprNodePtr condition_expr;

    constexpr GuardArm(ExprNodePtr condition_expr, StmtPtr block,
                       const Position &position) noexcept;
};

struct LiteralArm : public MatchArmBase

{
    std::vector<ExprNodePtr> literals;

    constexpr LiteralArm(std::vector<ExprNodePtr> literals, StmtPtr block,
                         const Position &position) noexcept;
};

bool operator==(const MatchArm &, const MatchArm &) noexcept;

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

bool operator==(const Program &, const Program &) noexcept;

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

// =========================================================
// =======================DEFINITIONS=======================
// =========================================================

// =================
// ===== TYPES =====
// =================

namespace
{
inline constexpr TypePtr clone_type_ptr(const TypePtr &type) noexcept
{
    return std::make_unique<Type>(*type);
}
} // namespace

inline constexpr SimpleType::SimpleType(const TypeEnum &type,
                                        const RefSpecifier &ref_spec) noexcept
    : type(type), ref_spec(ref_spec)
{
}

inline constexpr FunctionType::FunctionType(std::vector<TypePtr> arg_types,
                                            TypePtr return_type,
                                            const bool &is_const) noexcept
    : arg_types(std::move(arg_types)), return_type(std::move(return_type)),
      is_const(is_const)
{
}

inline constexpr FunctionType::FunctionType(const FunctionType &other) noexcept
    : arg_types(), return_type(clone_type_ptr(other.return_type)),
      is_const(other.is_const)
{
    for (const auto &arg : other.arg_types)
    {
        this->arg_types.push_back(clone_type_ptr(arg));
    }
}

// =======================
// ===== EXPRESSIONS =====
// =======================

inline constexpr VariableExpr::VariableExpr(const std::wstring &name,
                                            const Position &position) noexcept
    : AstNode(position), name(name)
{
}

inline constexpr BinaryExpr::BinaryExpr(ExprNodePtr lhs, ExprNodePtr rhs,
                                        const BinOpEnum &op,
                                        const Position &position) noexcept
    : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
{
}

inline constexpr UnaryExpr::UnaryExpr(ExprNodePtr expr, const UnaryOpEnum &op,
                                      const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)), op(op)
{
}

inline constexpr CallExpr::CallExpr(ExprNodePtr callable,
                                    std::vector<ExprNodePtr> args,
                                    const Position &position) noexcept
    : AstNode(position), callable(std::move(callable)), args(std::move(args))
{
}

inline constexpr LambdaCallExpr::LambdaCallExpr(
    ExprNodePtr callable, std::vector<ExprNodePtr> args,
    const Position &position) noexcept
    : AstNode(position), callable(std::move(callable)), args(std::move(args))
{
}

inline constexpr IndexExpr::IndexExpr(ExprNodePtr expr,
                                      ExprNodePtr index_value,
                                      const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)),
      index_value(std::move(index_value))
{
}

inline constexpr CastExpr::CastExpr(ExprNodePtr expr, TypePtr type,
                                    const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)), type(std::move(type))
{
}

inline constexpr U32Expr::U32Expr(const unsigned long long &value,
                                  const Position &position) noexcept
    : AstNode(position), value(value)
{
}

inline constexpr F64Expr::F64Expr(const double &value,
                                  const Position &position) noexcept
    : AstNode(position), value(value)
{
}

inline constexpr StringExpr::StringExpr(const std::wstring &value,
                                        const Position &position) noexcept
    : AstNode(position), value(value)
{
}

inline constexpr CharExpr::CharExpr(const wchar_t &value,
                                    const Position &position) noexcept
    : AstNode(position), value(value)
{
}

inline constexpr BoolExpr::BoolExpr(const bool &value,
                                    const Position &position) noexcept
    : AstNode(position), value(value)
{
}

inline constexpr Position get_expr_position(const ExprNode &expr)
{
    return std::visit(
        [](const AstNode &node) -> Position { return node.position; }, expr);
}

inline constexpr void set_expr_position(ExprNode &expr,
                                        const Position &position) noexcept
{
    std::visit([&position](AstNode &node) { node.position = position; }, expr);
}

// ======================
// ===== STATEMENTS =====
// ======================

inline constexpr ReturnStmt::ReturnStmt(const Position &position) noexcept
    : AstNode(position), expr()
{
}

inline constexpr ReturnStmt::ReturnStmt(ExprNodePtr expr,
                                        const Position &position) noexcept
    : AstNode(position), expr(std::move(expr))
{
}

inline constexpr ContinueStmt::ContinueStmt(const Position &position) noexcept
    : AstNode(position)
{
}

inline constexpr BreakStmt::BreakStmt(const Position &position) noexcept
    : AstNode(position)
{
}

inline constexpr AssignStmt::AssignStmt(ExprNodePtr lhs,
                                        const AssignType &type,
                                        ExprNodePtr rhs,
                                        const Position &position) noexcept
    : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), type(type)
{
}

inline constexpr ExprStmt::ExprStmt(ExprNodePtr expr,
                                    const Position &position) noexcept
    : AstNode(position), expr(std::move(expr))
{
}

inline constexpr WhileStmt::WhileStmt(ExprNodePtr condition_expr,
                                      StmtPtr statement,
                                      const Position &position) noexcept
    : AstNode(position), condition_expr(std::move(condition_expr)),
      statement(std::move(statement))
{
}

inline constexpr IfStmt::IfStmt(ExprNodePtr condition_expr, StmtPtr then_block,
                                StmtPtr else_block,
                                const Position &position) noexcept
    : AstNode(position), condition_expr(std::move(condition_expr)),
      then_block(std::move(then_block)), else_block(std::move(else_block))
{
}

inline constexpr MatchStmt::MatchStmt(ExprNodePtr matched_expr,
                                      std::vector<MatchArmPtr> match_arms,
                                      const Position &position) noexcept
    : AstNode(position), matched_expr(std::move(matched_expr)),
      match_arms(std::move(match_arms))
{
}

inline constexpr Block::Block(std::vector<StmtPtr> statements,
                              const Position &position) noexcept
    : AstNode(position), statements(std::move(statements))
{
}

inline constexpr VarDeclStmt::VarDeclStmt(const std::wstring &name,
                                          TypePtr type, ExprNodePtr value,
                                          const bool &is_mut,
                                          const Position &position) noexcept
    : AstNode(position), name(name), type(std::move(type)),
      initial_value(std::move(value)), is_mut(is_mut)
{
}

inline constexpr FuncDefStmt::FuncDefStmt(const std::wstring &name,
                                          std::vector<ParamPtr> params,
                                          TypePtr return_type, BlockPtr block,
                                          const bool &is_const,
                                          const Position &position) noexcept
    : AstNode(position), name(name), params(std::move(params)),
      return_type(std::move(return_type)), block(std::move(block)),
      is_const(is_const)
{
}

inline constexpr std::unique_ptr<FunctionType> FuncDefStmt::get_type()
    const noexcept
{
    std::vector<TypePtr> param_types;
    for (auto &param_ptr : this->params)
        param_types.push_back(clone_type_ptr(param_ptr->type));
    auto cloned_type = clone_type_ptr(this->return_type);
    return std::make_unique<FunctionType>(
        std::move(param_types), std::move(cloned_type), this->is_const);
}

inline constexpr ExternStmt::ExternStmt(const std::wstring &name,
                                        std::vector<ParamPtr> params,
                                        TypePtr return_type,
                                        const Position &position) noexcept
    : AstNode(position), name(name), params(std::move(params)),
      return_type(std::move(return_type))
{
}

inline constexpr std::unique_ptr<FunctionType> ExternStmt::get_type()
    const noexcept
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

inline constexpr MatchArmBase::MatchArmBase(StmtPtr block,
                                            const Position &position) noexcept
    : AstNode(position), block(std::move(block))
{
}

inline constexpr GuardArm::GuardArm(ExprNodePtr condition_expr, StmtPtr block,
                                    const Position &position) noexcept
    : MatchArmBase(std::move(block), position),
      condition_expr(std::move(condition_expr))
{
}

inline constexpr LiteralArm::LiteralArm(std::vector<ExprNodePtr> literals,
                                        StmtPtr block,
                                        const Position &position) noexcept
    : MatchArmBase(std::move(block), position), literals(std::move(literals))
{
}

inline constexpr ElseArm::ElseArm(StmtPtr block,
                                  const Position &position) noexcept
    : MatchArmBase(std::move(block), position)
{
}

// =====================
// ===== PARAMETER =====
// =====================

inline constexpr Parameter::Parameter(const std::wstring &name, TypePtr type,
                                      const Position &position) noexcept
    : AstNode(position), name(name), type(std::move(type))
{
}

bool operator==(const Parameter &, const Parameter &) noexcept;

// ===================
// ===== PROGRAM =====
// ===================

inline constexpr Program::Program(
    std::vector<std::unique_ptr<VarDeclStmt>> globals,
    std::vector<std::unique_ptr<FuncDefStmt>> functions,
    std::vector<std::unique_ptr<ExternStmt>> externs) noexcept
    : AstNode(Position(1, 1)), globals(std::move(globals)),
      functions(std::move(functions)), externs(std::move(externs))
{
}

#endif