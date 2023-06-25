#ifndef __OPERATORS_HPP__
#define __OPERATORS_HPP__
enum class BinaryOperator
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

enum class UnaryOperator
{
    INC,
    DEC,

    NEG,
    BIT_NEG,
};
#endif