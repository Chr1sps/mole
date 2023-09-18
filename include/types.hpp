#ifndef __TYPES_HPP__
#define __TYPES_HPP__
#include "visitor.hpp"
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
    virtual void accept(TypeVisitor &visitor) const = 0;

    virtual ~Type()
    {
    }
};

using TypePtr = std::shared_ptr<Type>;

struct SimpleType : public Type
{
    TypeEnum type;

    SimpleType(const TypeEnum &type) : type(type)
    {
    }

    void accept(TypeVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct NeverType : public Type
{
    NeverType() = default;

    void accept(TypeVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

struct FunctionType : public Type
{
    std::vector<TypePtr> arg_types;
    TypePtr return_type;
    bool is_const;

    FunctionType(const std::vector<TypePtr> &arg_types,
                 const TypePtr &return_type, const bool &is_const)
        : arg_types(std::move(arg_types)), return_type(std::move(return_type)),
          is_const(is_const)
    {
    }

    void accept(TypeVisitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

// visitors used for comparing types

bool operator==(const Type &first, const Type &other);

struct EquationVisitor;

struct SimpleTypeVisitor : public TypeVisitor
{
    bool value;

    SimpleTypeVisitor(const SimpleType &type) : type(type)
    {
    }

    void visit(const SimpleType &other) override;
    void visit(const NeverType &other) override;
    void visit(const FunctionType &other) override;

  private:
    const SimpleType &type;
};

struct NeverTypeVisitor : public TypeVisitor
{
    bool value;

    void visit(const SimpleType &other) override;
    void visit(const NeverType &other) override;
    void visit(const FunctionType &other) override;
};

struct FunctionTypeVisitor : public TypeVisitor
{
    bool value;

    FunctionTypeVisitor(const FunctionType &type) : type(type)
    {
    }

    void visit(const SimpleType &other) override;
    void visit(const NeverType &other) override;
    void visit(const FunctionType &other) override;

  private:
    const FunctionType &type;
};

struct EquationVisitor : public TypeVisitor
{

    bool value;

    EquationVisitor(const Type &type) : type(type)
    {
    }

    void visit(const SimpleType &other) override;
    void visit(const NeverType &other) override;
    void visit(const FunctionType &other) override;

  private:
    const Type &type;
};

#endif