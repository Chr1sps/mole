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
#include <vector>

struct AstNode
{

    Position position;

    virtual ~AstNode()
    {
    }

    virtual void accept(AstVisitor &visitor) const = 0;

  protected:
    AstNode(const Position &position) : position(position)
    {
    }
};

using AstNodePtr = std::unique_ptr<AstNode>;

struct ExprNode : public AstNode
{
  protected:
    ExprNode(const Position &position) : AstNode(position)
    {
    }
};

using ExprNodePtr = std::unique_ptr<ExprNode>;

struct VariableExpr : public ExprNode
{
    std::wstring name;

    VariableExpr(const std::wstring &name, const Position &position)
        : ExprNode(position), name(name)
    {
    }

    void accept(AstVisitor &visitor) const override
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
};

struct BinaryExpr : public ExprNode
{
    ExprNodePtr lhs, rhs;
    BinOpEnum op;

    BinaryExpr(ExprNodePtr &lhs, ExprNodePtr &rhs, const BinOpEnum &op,
               const Position &position)
        : ExprNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
    {
    }

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct UnaryExpr : public ExprNode
{
    ExprNodePtr expr;
    UnaryOpEnum op;

    UnaryExpr(ExprNodePtr &expr, const UnaryOpEnum &op,
              const Position &position)
        : ExprNode(position), expr(std::move(expr)), op(op)
    {
    }

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct CallExpr : public ExprNode
{
    ExprNodePtr callable;
    std::vector<ExprNodePtr> args;

    CallExpr(ExprNodePtr &callable, std::vector<ExprNodePtr> &args,
             const Position &position)
        : ExprNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    CallExpr(std::unique_ptr<VariableExpr> &callable,
             std::vector<ExprNodePtr> &args, const Position &position)
        : ExprNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct LambdaCallExpr : public ExprNode
{
    ExprNodePtr callable;
    std::vector<std::optional<ExprNodePtr>> args;

    LambdaCallExpr(ExprNodePtr &callable,
                   std::vector<std::optional<ExprNodePtr>> &args,
                   const Position &position)
        : ExprNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    LambdaCallExpr(std::unique_ptr<VariableExpr> &callable,
                   std::vector<std::optional<ExprNodePtr>> &args,
                   const Position &position)
        : ExprNode(position), callable(std::move(callable)),
          args(std::move(args))
    {
    }

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct ConstNode : public ExprNode
{
  protected:
    ConstNode(const Position &position) : ExprNode(position)
    {
    }
};

struct I32Expr : public ConstNode
{
    static const std::shared_ptr<SimpleType> type;
    unsigned long long value;

    I32Expr(const unsigned long long &value, const Position &position)
        : ConstNode(position), value(value)
    {
    }

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct F64Expr : public ConstNode
{
    static const std::shared_ptr<SimpleType> type;
    double value;

    F64Expr(const double &value, const Position &position)
        : ConstNode(position), value(value)
    {
    }

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct Statement : public AstNode
{
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

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

using BlockPtr = std::unique_ptr<Block>;

struct ReturnStmt : public Statement
{
    std::optional<ExprNodePtr> expr;

    ReturnStmt(const Position &position) : Statement(position), expr({})
    {
    }

    ReturnStmt(ExprNodePtr &expr, const Position &position)
        : Statement(position),
          expr(std::make_optional<ExprNodePtr>(std::move(expr)))
    {
    }

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct ContinueStmt : public Statement
{
    ContinueStmt(const Position &position) : Statement(position)
    {
    }
};

struct BreakStmt : public Statement
{
    BreakStmt(const Position &position) : Statement(position)
    {
    }
};

struct VarDeclStmt : public Statement
{
    std::wstring name;
    std::optional<TypePtr> type;
    std::optional<ExprNodePtr> initial_value;
    bool is_mut;

    VarDeclStmt(const std::wstring &name, std::optional<TypePtr> &type,
                std::optional<ExprNodePtr> &value, const bool &is_mut,
                const Position &position)
        : Statement(position), name(name), type(std::move(type)),
          initial_value(std::move(value)), is_mut(is_mut)
    {
    }

    void accept(AstVisitor &visitor) const override
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

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct WhileStmt : public Statement
{
    ExprNodePtr condition_expr;
    BlockPtr block;

    WhileStmt(ExprNodePtr &condition_expr, BlockPtr &block,
              const Position &position)
        : Statement(position), condition_expr(std::move(condition_expr)),
          block(std::move(block))
    {
    }
};

struct IfStmt : public Statement
{
    ExprNodePtr condition_expr;
    BlockPtr then_block;
    std::optional<BlockPtr> else_block;

    IfStmt(ExprNodePtr &condition_expr, BlockPtr &then_block,
           std::optional<BlockPtr> &else_block, const Position &position)
        : Statement(position), condition_expr(std::move(condition_expr)),
          then_block(std::move(then_block)), else_block(std::move(else_block))
    {
    }
};

struct MatchCase : public AstNode
{
    BlockPtr block;

    MatchCase(BlockPtr &block, const Position &position)
        : AstNode(position), block(std::move(block))
    {
    }
};

using MatchCasePtr = std::unique_ptr<MatchCase>;

struct PlaceholderCase : public MatchCase
{
    PlaceholderCase(BlockPtr &block, const Position &position)
        : MatchCase(block, position)
    {
    }
};

struct GuardCase : public MatchCase
{
    ExprNodePtr condition_expr;

    GuardCase(ExprNodePtr &condition_expr, BlockPtr &block,
              const Position &position)
        : MatchCase(block, position), condition_expr(std::move(condition_expr))
    {
    }
};

struct MatchStmt : public Statement
{
    ExprNodePtr matched_expr;
    std::vector<MatchCasePtr> match_cases;
};

struct Parameter : public AstNode
{
    std::wstring name;
    TypePtr type;

    Parameter(const std::wstring name, TypePtr &&type,
              const Position &position)
        : AstNode(position), name(name), type(std::move(type))
    {
    }

    void accept(AstVisitor &visitor) const override
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

    std::shared_ptr<FunctionType> get_type()
    {
        std::vector<TypePtr> param_types;
        for (auto &param_ptr : this->params)
            param_types.push_back(param_ptr->type);
        return std::make_shared<FunctionType>(param_types, this->return_type,
                                              this->is_const);
    }

    void accept(AstVisitor &visitor) const override
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

    std::shared_ptr<FunctionType> get_type()
    {
        std::vector<TypePtr> param_types;
        for (auto &param_ptr : this->params)
            param_types.push_back(param_ptr->type);
        return std::make_shared<FunctionType>(param_types, this->return_type,
                                              false);
    }

    void accept(AstVisitor &visitor) const override
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

    void accept(AstVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

using ProgramPtr = std::unique_ptr<Program>;
#endif