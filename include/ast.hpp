#ifndef __AST_HPP__
#define __AST_HPP__
#include "visitor.hpp"
#include <cstdint>
#include <memory>
#include <string>

struct AstNode
{
  public:
    virtual ~AstNode()
    {
    }

    virtual void accept(Visitor &visitor) const = 0;
};

struct ExprNode : public AstNode
{
};

using ExprNodePtr = std::unique_ptr<ExprNode>;

struct VariableExprNode : public ExprNode
{
    std::string name;

    VariableExprNode(const std::string &name) : name(name)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct BinaryExprNode : public ExprNode
{
};

struct ConstNode : public AstNode
{
};

struct I32ConstNode : public ConstNode
{
    int32_t value;

    I32ConstNode(const int32_t &value) : value(value)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct F32ConstNode : public ConstNode
{
    float value;

    F32ConstNode(const float &value) : value(value)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct F64ConstNode : public ConstNode
{
    double value;

    F64ConstNode(const double &value) : value(value)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};
#endif