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

struct ExprNode : public AstNode
{
    virtual void accept(ExprVisitor &visitor) const = 0;

  protected:
    ExprNode(const Position &position) : AstNode(position)
    {
    }
};

// using ExprNodeVariant = std::unique_ptr<ExprNode>;

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

using ExprNodeVariant =
    std::variant<std::monostate, VariableExprPtr, BinaryExprPtr, UnaryExprPtr,
                 CallExprPtr, LambdaCallExprPtr, IndexExprPtr, CastExprPtr,
                 I32ExprPtr, F64ExprPtr, BoolExprPtr, StringExprPtr,
                 CharExprPtr>;

template <typename... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct PositionVisitor
{
    Position &operator()(const AstNodePtr &node)
    {
        return node->position;
    }

    Position &operator()(const std::monostate &)
    {
        throw std::exception();
    }
};

Position &get_position(const ExprNodeVariant &value)
{
    // Position result(1, 1);
    // std::visit(overloaded{[&result](const std::unique_ptr<AstNode> &node) {
    //                           result = node->position;
    //                       },
    //                       [](const std::monostate &) {}},
    //            value);
    // return result;
    return std::visit(PositionVisitor(), value);
}

struct VariableExpr : public ExprNode
{
    std::wstring name;

    VariableExpr(const std::wstring &name, const Position &position)
        : ExprNode(position), name(name)
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
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

struct BinaryExpr : public ExprNode
{
    ExprNodeVariant lhs, rhs;
    BinOpEnum op;

    BinaryExpr(ExprNodeVariant &lhs, ExprNodeVariant &rhs, const BinOpEnum &op,
               const Position &position)
        : ExprNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
    {
    }

    BinaryExpr(ExprNodeVariant &&lhs, ExprNodeVariant &&rhs,
               const BinOpEnum &op, const Position &position)
        : ExprNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct UnaryExpr : public ExprNode
{
    ExprNodeVariant expr;
    UnaryOpEnum op;

    UnaryExpr(ExprNodeVariant &expr, const UnaryOpEnum &op,
              const Position &position)
        : ExprNode(position), expr(std::move(expr)), op(op)
    {
    }

    UnaryExpr(ExprNodeVariant &&expr, const UnaryOpEnum &op,
              const Position &position)
        : ExprNode(position), expr(std::move(expr)), op(op)
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct CallExpr : public ExprNode
{
    ExprNodeVariant callable;
    std::vector<ExprNodeVariant> args;

    CallExpr(ExprNodeVariant &callable, std::vector<ExprNodeVariant> &args,
             const Position &position)
        : ExprNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    CallExpr(ExprNodeVariant &&callable, std::vector<ExprNodeVariant> &&args,
             const Position &position)
        : ExprNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct LambdaCallExpr : public ExprNode
{
    ExprNodeVariant callable;
    std::vector<ExprNodeVariant> args;

    LambdaCallExpr(ExprNodeVariant &callable,
                   std::vector<ExprNodeVariant> &args,
                   const Position &position)
        : ExprNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    LambdaCallExpr(ExprNodeVariant &&callable,
                   std::vector<ExprNodeVariant> &&args,
                   const Position &position)
        : ExprNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct IndexExpr : public ExprNode
{
    ExprNodeVariant expr, index_value;

    IndexExpr(ExprNodeVariant &expr, ExprNodeVariant &index_value,
              const Position &position)
        : ExprNode(position), expr(std::move(expr)),
          index_value(std::move(index_value))
    {
    }

    IndexExpr(ExprNodeVariant &&expr, ExprNodeVariant &&index_value,
              const Position &position)
        : ExprNode(position), expr(std::move(expr)),
          index_value(std::move(index_value))
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct CastExpr : public ExprNode
{
    ExprNodeVariant expr;
    TypePtr type;

    CastExpr(ExprNodeVariant &expr, TypePtr &type, const Position &position)
        : ExprNode(position), expr(std::move(expr)), type(std::move(type))
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct I32Expr : public ExprNode
{
    static const std::shared_ptr<SimpleType> type;
    unsigned long long value;

    I32Expr(const unsigned long long &value, const Position &position)
        : ExprNode(position), value(value)
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct F64Expr : public ExprNode
{
    static const std::shared_ptr<SimpleType> type;
    double value;

    F64Expr(const double &value, const Position &position)
        : ExprNode(position), value(value)
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct StringExpr : public ExprNode
{
    // static const std::shared_ptr<SimpleType> type;
    std::wstring value;

    StringExpr(const std::wstring &value, const Position &position)
        : ExprNode(position), value(value)
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct CharExpr : public ExprNode
{
    // static const std::shared_ptr<SimpleType> type;
    wchar_t value;

    CharExpr(const wchar_t &value, const Position &position)
        : ExprNode(position), value(value)
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct BoolExpr : public ExprNode
{
    // static const std::shared_ptr<SimpleType> type;
    bool value;

    BoolExpr(const bool &value, const Position &position)
        : ExprNode(position), value(value)
    {
    }

    void accept(ExprVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

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
    ExprNodeVariant expr;

    ReturnStmt(const Position &position) : Statement(position), expr()
    {
    }

    ReturnStmt(ExprNodeVariant &expr, const Position &position)
        : Statement(position), expr(std::move(expr))
    {
    }

    ReturnStmt(ExprNodeVariant &&expr, const Position &position)
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
    ExprNodeVariant initial_value;
    bool is_mut;

    VarDeclStmt(const std::wstring &name, TypePtr &type,
                ExprNodeVariant &value, const bool &is_mut,
                const Position &position)
        : Statement(position), name(name), type(std::move(type)),
          initial_value(std::move(value)), is_mut(is_mut)
    {
    }

    VarDeclStmt(std::wstring &&name, TypePtr &&type, ExprNodeVariant &&value,
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
    ExprNodeVariant lhs;
    AssignType type;
    ExprNodeVariant rhs;

    AssignStmt(ExprNodeVariant &lhs, const AssignType &type,
               ExprNodeVariant &rhs, const Position &position)
        : Statement(position), lhs(std::move(lhs)), type(type),
          rhs(std::move(rhs))
    {
    }

    AssignStmt(ExprNodeVariant &&lhs, const AssignType &type,
               ExprNodeVariant &&rhs, const Position &position)
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
    ExprNodeVariant expr;

    ExprStmt(ExprNodeVariant &expr, const Position &position)
        : Statement(position), expr(std::move(expr))
    {
    }

    ExprStmt(ExprNodeVariant &&expr, const Position &position)
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
    ExprNodeVariant condition_expr;
    StmtPtr statement;

    WhileStmt(ExprNodeVariant &condition_expr, StmtPtr &statement,
              const Position &position)
        : Statement(position), condition_expr(std::move(condition_expr)),
          statement(std::move(statement))
    {
    }

    WhileStmt(ExprNodeVariant &&condition_expr, StmtPtr &&statement,
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
    ExprNodeVariant condition_expr;
    StmtPtr then_block;
    StmtPtr else_block;

    IfStmt(ExprNodeVariant &condition_expr, StmtPtr &then_block,
           StmtPtr &else_block, const Position &position)
        : Statement(position), condition_expr(std::move(condition_expr)),
          then_block(std::move(then_block)), else_block(std::move(else_block))
    {
    }

    IfStmt(ExprNodeVariant &&condition_expr, StmtPtr &&then_block,
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
    ExprNodeVariant condition_expr;

    GuardArm(ExprNodeVariant &condition_expr, StmtPtr &block,
             const Position &position)
        : MatchArm(block, position), condition_expr(std::move(condition_expr))
    {
    }

    GuardArm(ExprNodeVariant &&condition_expr, StmtPtr &&block,
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
    std::vector<ExprNodeVariant> literals;

    LiteralArm(std::vector<ExprNodeVariant> &literals, StmtPtr &block,
               const Position &position)
        : MatchArm(block, position), literals(std::move(literals))
    {
    }

    LiteralArm(std::vector<ExprNodeVariant> &&literals, StmtPtr &&block,
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
    ExprNodeVariant matched_expr;
    std::vector<MatchArmPtr> match_arms;

    MatchStmt(ExprNodeVariant &matched_expr,
              std::vector<MatchArmPtr> &match_arms, const Position &position)
        : Statement(position), matched_expr(std::move(matched_expr)),
          match_arms(std::move(match_arms))
    {
    }

    MatchStmt(ExprNodeVariant &&matched_expr,
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