#ifndef __OPERATORS_HPP__
#define __OPERATORS_HPP__
#include "visitor.hpp"
#include <memory>
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

struct Operator
{

    virtual void accept(BinOpVisitor &visitor) = 0;

    virtual ~Operator()
    {
    }
};

struct BinaryOperator : public Operator
{
    unsigned precedence;

    BinaryOperator(const unsigned &precedence) : precedence(precedence)
    {
    }
};

using BinaryPtr = std::shared_ptr<BinaryOperator>;

struct BuiltInBinOp : public BinaryOperator
{
    BinOpEnum op;

    BuiltInBinOp(const unsigned &precedence, const BinOpEnum &op)
        : BinaryOperator(precedence), op(op)
    {
    }

    void accept(BinOpVisitor &visitor) override
    {
        visitor.visit(*this);
    }
};

struct UnaryOperator : public Operator
{
};

using UnaryPtr = std::shared_ptr<UnaryOperator>;

struct BuiltInUnaryOp : public UnaryOperator
{
    UnaryOpEnum op;

    BuiltInUnaryOp(const UnaryOpEnum &op) : op(op)
    {
    }

    void accept(BinOpVisitor &visitor) override
    {
        visitor.visit(*this);
    }
};
#endif