#ifndef __TYPES_HPP__
#define __TYPES_HPP__
#include <memory>
#include <vector>
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
  protected:
    Type() = default;

  public:
    virtual void accept(Visitor &visitor) = 0;

    virtual ~Type()
    {
    }
};

struct SimpleType : public Type
{
    TypeEnum type;

    SimpleType(const TypeEnum &type) : type(type)
    {
    }

    void accept(Visitor &visitor) override
    {
        visitor.visit(*this);
    }
};

struct NeverType : public Type
{
    NeverType() = default;

    void accept(Visitor &visitor) override
    {
        visitor.visit(*this);
    }
};

using TypePtr = std::unique_ptr<Type>;

struct FunctionType : public Type
{
    std::vector<TypePtr> arg_types;
    TypePtr return_type;

    FunctionType(std::vector<TypePtr> &arg_types, TypePtr &return_type)
        : arg_types(std::move(arg_types)), return_type(std::move(return_type))
    {
    }

    void accept(Visitor &visitor) override
    {
        visitor.visit(*this);
    }
};
#endif