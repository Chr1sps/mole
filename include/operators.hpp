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
    unsigned precedence;

    Operator(const unsigned &precedence) : precedence(precedence)
    {
    }

    virtual void accept(Visitor &visitor) = 0;

    virtual ~Operator()
    {
    }
};

struct BinaryOperator : public Operator
{
    BinaryOperator(const unsigned &precedence) : Operator(precedence)
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

    void accept(Visitor &visitor) override
    {
        visitor.visit(*this);
    }
};

struct UnaryOperator : public Operator
{
    UnaryOperator(const unsigned &precedence) : Operator(precedence)
    {
    }
};

using UnaryPtr = std::shared_ptr<UnaryOperator>;

struct BuiltInUnaryOp : public UnaryOperator
{
    UnaryOpEnum op;

    BuiltInUnaryOp(const unsigned &precedence, const UnaryOpEnum &op)
        : UnaryOperator(precedence), op(op)
    {
    }

    void accept(Visitor &visitor) override
    {
        visitor.visit(*this);
    }
};
#endif