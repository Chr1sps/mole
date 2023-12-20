#ifndef __AST_HPP__
#define __AST_HPP__
#include "position.hpp"
#include "types.hpp"
#include "visitor.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct AstNode
{

    Position position;

    virtual ~AstNode()
    {
    }

  protected:
    AstNode(const Position &position) : position(position)
    {
    }
};

using AstNodePtr = std::unique_ptr<AstNode>;

using VariableExprPtr = std::unique_ptr<VariableExpr>;
using BinaryExprPtr = std::unique_ptr<BinaryExpr>;
using UnaryExprPtr = std::unique_ptr<UnaryExpr>;
using CallExprPtr = std::unique_ptr<CallExpr>;
using LambdaCallExprPtr = std::unique_ptr<LambdaCallExpr>;
using IndexExprPtr = std::unique_ptr<IndexExpr>;
using CastExprPtr = std::unique_ptr<CastExpr>;
using I32ExprPtr = std::unique_ptr<I32Expr>;
using F64ExprPtr = std::unique_ptr<F64Expr>;
using StringExprPtr = std::unique_ptr<StringExpr>;
using CharExprPtr = std::unique_ptr<CharExpr>;
using BoolExprPtr = std::unique_ptr<BoolExpr>;

using ExprNode = std::variant<VariableExpr, BinaryExpr, UnaryExpr, CallExpr,
                              LambdaCallExpr, IndexExpr, CastExpr, I32Expr,
                              F64Expr, BoolExpr, StringExpr, CharExpr>;
using ExprNodePtr = std::unique_ptr<ExprNode>;

template <typename... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct VariableExpr : public AstNode
{
    std::wstring name;

    VariableExpr(const std::wstring &name, const Position &position)
        : AstNode(position), name(name)
    {
    }
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

    BinaryExpr(ExprNodePtr &lhs, ExprNodePtr &rhs, const BinOpEnum &op,
               const Position &position)
        : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
    {
    }

    BinaryExpr(ExprNodePtr &&lhs, ExprNodePtr &&rhs, const BinOpEnum &op,
               const Position &position)
        : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
    {
    }
};

struct UnaryExpr : public AstNode
{
    ExprNodePtr expr;
    UnaryOpEnum op;

    UnaryExpr(ExprNodePtr &expr, const UnaryOpEnum &op,
              const Position &position)
        : AstNode(position), expr(std::move(expr)), op(op)
    {
    }

    UnaryExpr(ExprNodePtr &&expr, const UnaryOpEnum &op,
              const Position &position)
        : AstNode(position), expr(std::move(expr)), op(op)
    {
    }
};

struct CallExpr : public AstNode
{
    ExprNodePtr callable;
    std::vector<ExprNodePtr> args;

    CallExpr(ExprNodePtr &callable, std::vector<ExprNodePtr> &args,
             const Position &position)
        : AstNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    CallExpr(ExprNodePtr &&callable, std::vector<ExprNodePtr> &&args,
             const Position &position)
        : AstNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }
};

struct LambdaCallExpr : public AstNode
{
    ExprNodePtr callable;
    std::vector<ExprNodePtr> args;

    LambdaCallExpr(ExprNodePtr &callable, std::vector<ExprNodePtr> &args,
                   const Position &position)
        : AstNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    LambdaCallExpr(ExprNodePtr &&callable, std::vector<ExprNodePtr> &&args,
                   const Position &position)
        : AstNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }
};

struct IndexExpr : public AstNode
{
    ExprNodePtr expr, index_value;

    IndexExpr(ExprNodePtr &expr, ExprNodePtr &index_value,
              const Position &position)
        : AstNode(position), expr(std::move(expr)),
          index_value(std::move(index_value))
    {
    }

    IndexExpr(ExprNodePtr &&expr, ExprNodePtr &&index_value,
              const Position &position)
        : AstNode(position), expr(std::move(expr)),
          index_value(std::move(index_value))
    {
    }
};

struct CastExpr : public AstNode
{
    ExprNodePtr expr;
    TypePtr type;

    CastExpr(ExprNodePtr &expr, TypePtr &type, const Position &position)
        : AstNode(position), expr(std::move(expr)), type(std::move(type))
    {
    }
};

struct I32Expr : public AstNode
{
    static const std::shared_ptr<SimpleType> type;
    unsigned long long value;

    I32Expr(const unsigned long long &value, const Position &position)
        : AstNode(position), value(value)
    {
    }
};

struct F64Expr : public AstNode
{
    static const std::shared_ptr<SimpleType> type;
    double value;

    F64Expr(const double &value, const Position &position)
        : AstNode(position), value(value)
    {
    }
};

struct StringExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    std::wstring value;

    StringExpr(const std::wstring &value, const Position &position)
        : AstNode(position), value(value)
    {
    }
};

struct CharExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    wchar_t value;

    CharExpr(const wchar_t &value, const Position &position)
        : AstNode(position), value(value)
    {
    }
};

struct BoolExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    bool value;

    BoolExpr(const bool &value, const Position &position)
        : AstNode(position), value(value)
    {
    }
};

inline Position get_expr_position(const ExprNode &variant)
{
    return std::visit(
        [](const AstNode &node) -> Position { return node.position; },
        variant);
}

inline void set_expr_position(ExprNode &variant, const Position &position)
{
    std::visit([&position](AstNode &node) { node.position = position; },
               variant);
}

struct Statement : public AstNode
{
    virtual void accept(StmtVisitor &visitor) const = 0;

  protected:
    // Position position;

    Statement(const Position &position) : AstNode(position)
    {
    }
};

using StmtPtr = std::unique_ptr<Statement>;

struct Block : public Statement
{
    std::vector<StmtPtr> statements;

    Block(std::vector<StmtPtr> &statements, const Position &position)
        : Statement(position), statements(std::move(statements))
    {
    }

    Block(std::vector<StmtPtr> &&statements, const Position &position)
        : Statement(position), statements(std::move(statements))
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

using BlockPtr = std::unique_ptr<Block>;

struct ReturnStmt : public Statement
{
    ExprNodePtr expr;

    ReturnStmt(const Position &position) : Statement(position), expr()
    {
    }

    ReturnStmt(ExprNodePtr &expr, const Position &position)
        : Statement(position), expr(std::move(expr))
    {
    }

    ReturnStmt(ExprNodePtr &&expr, const Position &position)
        : Statement(position), expr(std::move(expr))
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct ContinueStmt : public Statement
{
    ContinueStmt(const Position &position) : Statement(position)
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct BreakStmt : public Statement
{
    BreakStmt(const Position &position) : Statement(position)
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct VarDeclStmt : public Statement
{
    std::wstring name;
    TypePtr type;
    ExprNodePtr initial_value;
    bool is_mut;

    VarDeclStmt(const std::wstring &name, TypePtr &type, ExprNodePtr &value,
                const bool &is_mut, const Position &position)
        : Statement(position), name(name), type(std::move(type)),
          initial_value(std::move(value)), is_mut(is_mut)
    {
    }

    VarDeclStmt(std::wstring &&name, TypePtr &&type, ExprNodePtr &&value,
                bool &&is_mut, Position &&position)
        : Statement(position), name(name), type(std::move(type)),
          initial_value(std::move(value)), is_mut(is_mut)
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
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

struct AssignStmt : public Statement
{
    ExprNodePtr lhs;
    AssignType type;
    ExprNodePtr rhs;

    AssignStmt(ExprNodePtr &lhs, const AssignType &type, ExprNodePtr &rhs,
               const Position &position)
        : Statement(position), lhs(std::move(lhs)), type(type),
          rhs(std::move(rhs))
    {
    }

    AssignStmt(ExprNodePtr &&lhs, const AssignType &type, ExprNodePtr &&rhs,
               const Position &position)
        : Statement(position), lhs(std::move(lhs)), type(type),
          rhs(std::move(rhs))
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct ExprStmt : public Statement
{
    ExprNodePtr expr;

    ExprStmt(ExprNodePtr &expr, const Position &position)
        : Statement(position), expr(std::move(expr))
    {
    }

    ExprStmt(ExprNodePtr &&expr, const Position &position)
        : Statement(position), expr(std::move(expr))
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct WhileStmt : public Statement
{
    ExprNodePtr condition_expr;
    StmtPtr statement;

    WhileStmt(ExprNodePtr &condition_expr, StmtPtr &statement,
              const Position &position)
        : Statement(position), condition_expr(std::move(condition_expr)),
          statement(std::move(statement))
    {
    }

    WhileStmt(ExprNodePtr &&condition_expr, StmtPtr &&statement,
              const Position &position)
        : Statement(position), condition_expr(std::move(condition_expr)),
          statement(std::move(statement))
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct IfStmt : public Statement
{
    ExprNodePtr condition_expr;
    StmtPtr then_block;
    StmtPtr else_block;

    IfStmt(ExprNodePtr &condition_expr, StmtPtr &then_block,
           StmtPtr &else_block, const Position &position)
        : Statement(position), condition_expr(std::move(condition_expr)),
          then_block(std::move(then_block)), else_block(std::move(else_block))
    {
    }

    IfStmt(ExprNodePtr &&condition_expr, StmtPtr &&then_block,
           StmtPtr &&else_block, const Position &position)
        : Statement(position), condition_expr(std::move(condition_expr)),
          then_block(std::move(then_block)), else_block(std::move(else_block))
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct MatchArm : public AstNode
{
    StmtPtr block;

    MatchArm(StmtPtr &block, const Position &position)
        : AstNode(position), block(std::move(block))
    {
    }

    virtual void accept(MatchArmVisitor &visitor) const = 0;
};

using MatchArmPtr = std::unique_ptr<MatchArm>;

struct ElseArm : public MatchArm
{
    ElseArm(StmtPtr &block, const Position &position)
        : MatchArm(block, position)
    {
    }

    ElseArm(StmtPtr &&block, const Position &position)
        : MatchArm(block, position)
    {
    }

    void accept(MatchArmVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct GuardArm : public MatchArm
{
    ExprNodePtr condition_expr;

    GuardArm(ExprNodePtr &condition_expr, StmtPtr &block,
             const Position &position)
        : MatchArm(block, position), condition_expr(std::move(condition_expr))
    {
    }

    GuardArm(ExprNodePtr &&condition_expr, StmtPtr &&block,
             const Position &position)
        : MatchArm(block, position), condition_expr(std::move(condition_expr))
    {
    }

    void accept(MatchArmVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct LiteralArm : public MatchArm
{
    std::vector<ExprNodePtr> literals;

    LiteralArm(std::vector<ExprNodePtr> &literals, StmtPtr &block,
               const Position &position)
        : MatchArm(block, position), literals(std::move(literals))
    {
    }

    LiteralArm(std::vector<ExprNodePtr> &&literals, StmtPtr &&block,
               const Position &position)
        : MatchArm(block, position), literals(std::move(literals))
    {
    }

    void accept(MatchArmVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct MatchStmt : public Statement
{
    ExprNodePtr matched_expr;
    std::vector<MatchArmPtr> match_arms;

    MatchStmt(ExprNodePtr &matched_expr, std::vector<MatchArmPtr> &match_arms,
              const Position &position)
        : Statement(position), matched_expr(std::move(matched_expr)),
          match_arms(std::move(match_arms))
    {
    }

    MatchStmt(ExprNodePtr &&matched_expr,
              std::vector<MatchArmPtr> &&match_arms, const Position &position)
        : Statement(position), matched_expr(std::move(matched_expr)),
          match_arms(std::move(match_arms))
    {
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct Parameter : public AstNode
{
    std::wstring name;
    TypePtr type;

    Parameter(const std::wstring &name, TypePtr &&type,
              const Position &position)
        : AstNode(position), name(name), type(std::move(type))
    {
    }

    void accept(AstVisitor &visitor) const
    {
        visitor.visit(*this);
    }
};

using ParamPtr = std::unique_ptr<Parameter>;

struct FuncDefStmt : public Statement
{
    std::wstring name;
    std::vector<ParamPtr> params;
    TypePtr return_type;
    BlockPtr block;
    bool is_const;

    FuncDefStmt(const std::wstring &name, std::vector<ParamPtr> &params,
                TypePtr &return_type, BlockPtr &block, const bool &is_const,
                const Position &position)
        : Statement(position), name(name), params(std::move(params)),
          return_type(std::move(return_type)), block(std::move(block)),
          is_const(is_const)
    {
    }

    FuncDefStmt(const std::wstring &name, std::vector<ParamPtr> &&params,
                TypePtr &&return_type, BlockPtr &&block, const bool &is_const,
                const Position &position)
        : Statement(position), name(name), params(std::move(params)),
          return_type(std::move(return_type)), block(std::move(block)),
          is_const(is_const)
    {
    }

    std::shared_ptr<FunctionType> get_type()
    {
        std::vector<TypePtr> param_types;
        for (auto &param_ptr : this->params)
            param_types.push_back(param_ptr->type);
        return std::make_shared<FunctionType>(param_types, this->return_type,
                                              this->is_const);
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct ExternStmt : public Statement
{
    std::wstring name;
    std::vector<ParamPtr> params;
    TypePtr return_type;

    ExternStmt(const std::wstring &name, std::vector<ParamPtr> &params,
               TypePtr &return_type, const Position &position)
        : Statement(position), name(name), params(std::move(params)),
          return_type(std::move(return_type))
    {
    }

    ExternStmt(const std::wstring &name, std::vector<ParamPtr> &&params,
               TypePtr &&return_type, const Position &position)
        : Statement(position), name(name), params(std::move(params)),
          return_type(std::move(return_type))
    {
    }

    std::shared_ptr<FunctionType> get_type()
    {
        std::vector<TypePtr> param_types;
        for (auto &param_ptr : this->params)
            param_types.push_back(param_ptr->type);
        return std::make_shared<FunctionType>(param_types, this->return_type,
                                              false);
    }

    void accept(StmtVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct Program : public AstNode
{
    std::vector<std::unique_ptr<VarDeclStmt>> globals;
    std::vector<std::unique_ptr<FuncDefStmt>> functions;
    std::vector<std::unique_ptr<ExternStmt>> externs;

    Program(std::vector<std::unique_ptr<VarDeclStmt>> &globals,
            std::vector<std::unique_ptr<FuncDefStmt>> &functions,
            std::vector<std::unique_ptr<ExternStmt>> &externs)
        : AstNode(Position(1, 1)), globals(std::move(globals)),
          functions(std::move(functions)), externs(std::move(externs))
    {
    }

    Program(std::vector<std::unique_ptr<VarDeclStmt>> &&globals,
            std::vector<std::unique_ptr<FuncDefStmt>> &&functions,
            std::vector<std::unique_ptr<ExternStmt>> &&externs)
        : AstNode(Position(1, 1)), globals(std::move(globals)),
          functions(std::move(functions)), externs(std::move(externs))
    {
    }

    void accept(AstVisitor &visitor) const
    {
        visitor.visit(*this);
    }
};

using ProgramPtr = std::unique_ptr<Program>;

#endif