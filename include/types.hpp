#ifndef __TYPES_HPP__
#define __TYPES_HPP__
enum class TypeEnum
{
    BOOL,

    I8,
    I16,
    I32,
    I64,

    U8,
    U16,
    U32,
    U64,

    F32,
    F64,

    CHAR,
};

struct Type
{
};

struct SimpleType : public Type
{
    TypeEnum type;
};

struct NeverType : public Type
{
};

using TypePtr = std::unique_ptr<Type>;

struct FunctionType : public Type
{
    std::vector<TypePtr> arg_types;
    TypePtr return_type;
};
#endif