#ifndef __AST_HPP__
#define __AST_HPP__
#include "operators.hpp"
#include "types.hpp"
#include "visitor.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct AstNode
{
  public:
    virtual ~AstNode()
    {
    }

    virtual void accept(Visitor &visitor) const = 0;
};

using AstNodePtr = std::unique_ptr<AstNodePtr>;

struct ExprNode : public AstNode
{
};

using ExprNodePtr = std::unique_ptr<ExprNode>;

struct VariableExpr : public ExprNode
{
    std::string name;

    VariableExpr(const std::string &name) : name(name)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct BinaryExpr : public ExprNode
{
    ExprNodePtr lhs, rhs;
    BinaryOperator op;

    BinaryExpr(ExprNodePtr &lhs, ExprNodePtr &rhs, const BinaryOperator &op)
        : lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct UnaryExpr : public ExprNode
{
    ExprNodePtr expr;
    UnaryOperator op;

    UnaryExpr(ExprNodePtr &expr, UnaryOperator op)
        : expr(std::move(expr)), op(op)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct CallExpr : public ExprNode
{
    std::string func_name;
    std::vector<ExprNodePtr> args;

    CallExpr(const std::string &func_name, std::vector<ExprNodePtr> &args)
        : func_name(func_name), args(std::move(args))
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct ConstNode : public ExprNode
{
};

struct I32Expr : public ConstNode
{
    int32_t value;

    I32Expr(const int32_t &value) : value(value)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct F32Expr : public ConstNode
{
    float value;

    F32Expr(const float &value) : value(value)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct F64Expr : public ConstNode
{
    double value;

    F64Expr(const double &value) : value(value)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct Statement : public AstNode
{
};

using StmtPtr = std::unique_ptr<Statement>;

struct Block : public AstNode
{
    std::vector<StmtPtr> statements;

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

using BlockPtr = std::unique_ptr<Block>;

struct ReturnStmt : public Statement

{
    ExprNodePtr expr;

    ReturnStmt(ExprNodePtr &expr) : expr(std::move(expr))
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct VarDeclStmt : public Statement
{
    std::string name;
    std::optional<Type> type;
    std::optional<ExprNodePtr> initial_value;

    VarDeclStmt(Type &type, ExprNodePtr &value)
        : type(std::move(type)), initial_value(std::move(value))
    {
    }

    VarDeclStmt(Type &type)
        : type(std::move(type)), initial_value(std::nullopt)
    {
    }

    VarDeclStmt(ExprNodePtr &value)
        : type(std::nullopt), initial_value(std::move(value))
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct AssignStmt : public Statement
{
    std::string name;
    ExprNodePtr value;

    AssignStmt(const std::string &name, ExprNodePtr &value)
        : name(name), value(std::move(value))
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct Parameter
{
    std::string name;
    TypePtr type;
};

using ParamPtr = std::unique_ptr<Parameter>;

struct FuncDefStmt : public Statement
{
    std::string name;
    std::vector<ParamPtr> params;
    TypePtr return_type;
    BlockPtr block;

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct ExternStmt : public Statement
{
    std::string name;
    std::vector<ParamPtr> params;
    TypePtr return_type;

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct Program : public AstNode
{
    std::vector<VarDeclStmt> globals;
    std::vector<FuncDefStmt> functions;
    std::vector<ExternStmt> externs;

    Program(std::vector<VarDeclStmt> globals,
            std::vector<FuncDefStmt> functions,
            std::vector<ExternStmt> externs)
        : globals(std::move(globals)), functions(std::move(functions)),
          externs(std::move(externs))
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

using ProgramPtr = std::unique_ptr<Program>;
// struct
#endif