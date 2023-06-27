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

using AstNodePtr = std::unique_ptr<AstNode>;

struct ExprNode : public AstNode
{
};

using ExprNodePtr = std::unique_ptr<ExprNode>;

struct VariableExpr : public ExprNode
{
    std::wstring name;

    VariableExpr(const std::wstring &name) : name(name)
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
    BinaryPtr op;

    BinaryExpr(ExprNodePtr &lhs, ExprNodePtr &rhs, const BinaryPtr &op)
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
    UnaryPtr op;

    UnaryExpr(ExprNodePtr &expr, UnaryPtr op) : expr(std::move(expr)), op(op)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct CallExpr : public ExprNode
{
    std::wstring func_name;
    std::vector<ExprNodePtr> args;

    CallExpr(const std::wstring &func_name, std::vector<ExprNodePtr> &args)
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

struct Block : public Statement
{
    std::vector<StmtPtr> statements;

    Block(std::vector<StmtPtr> &statements) : statements(std::move(statements))
    {
    }

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
    std::wstring name;
    std::optional<TypePtr> type;
    std::optional<ExprNodePtr> initial_value;

    VarDeclStmt(const std::wstring &name, std::optional<TypePtr> &type,
                std::optional<ExprNodePtr> &value)
        : name(name), type(std::move(type)), initial_value(std::move(value))
    {
    }

    VarDeclStmt(const std::wstring &name, TypePtr &type, ExprNodePtr &value)
        : name(name), type(std::move(type)), initial_value(std::move(value))
    {
    }

    VarDeclStmt(const std::wstring &name, TypePtr &type)
        : name(name), type(std::move(type)), initial_value({})
    {
    }

    VarDeclStmt(const std::wstring &name, ExprNodePtr &value)
        : name(name), type({}), initial_value(std::move(value))
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
    std::wstring name;
    TypePtr type;

    Parameter(const std::wstring name, TypePtr &&type)
        : name(name), type(std::move(type))
    {
    }
};

using ParamPtr = std::unique_ptr<Parameter>;

struct FuncDefStmt : public Statement
{
    std::wstring name;
    std::vector<ParamPtr> params;
    TypePtr return_type;
    BlockPtr block;

    FuncDefStmt(const std::wstring &name, std::vector<ParamPtr> &params,
                TypePtr &return_type, BlockPtr &block)
        : name(name), params(std::move(params)),
          return_type(std::move(return_type)), block(std::move(block))
    {
    }

    void accept(Visitor &visitor) const override
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
               TypePtr &return_type)
        : name(name), params(std::move(params)),
          return_type(std::move(return_type))
    {
    }

    void accept(Visitor &visitor) const override
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