#ifndef __TYPES_HPP__
#define __TYPES_HPP__
#include "visitor.hpp"
#include <memory>
#include <vector>
enum class TypeEnum
{
    BOOL,

    I32,

    U32,

    F64,

    CHAR,
    STR,
};

enum class RefSpecifier
{
    NON_REF,
    REF,
    MUT_REF
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
    RefSpecifier ref_spec;

    SimpleType(const TypeEnum &type, const RefSpecifier &ref_spec)
        : type(type), ref_spec(ref_spec)
    {
    }

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

    FunctionType(std::vector<TypePtr> &&arg_types, TypePtr &&return_type,
                 const bool &is_const)
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

struct TypeEquationVisitor;

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

struct TypeEquationVisitor : public TypeVisitor
{

    bool value;

    TypeEquationVisitor(const Type &type) : type(type)
    {
    }

    void visit(const SimpleType &other) override;
    void visit(const NeverType &other) override;
    void visit(const FunctionType &other) override;

  private:
    const Type &type;
};

#endif