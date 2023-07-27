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
    virtual void accept(Visitor &visitor) const = 0;

    virtual ~Type()
    {
    }
};

using TypePtr = std::unique_ptr<Type>;

struct SimpleType : public Type
{
    TypeEnum type;

    SimpleType(const TypeEnum &type) : type(type)
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

using SimpleTypePtr = std::unique_ptr<SimpleType>;

struct NeverType : public Type
{
    NeverType() = default;

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

using NeverTypePtr = std::unique_ptr<NeverType>;

struct FunctionType : public Type
{
    std::vector<TypePtr> arg_types;
    TypePtr return_type;

    FunctionType(std::vector<TypePtr> &arg_types, TypePtr &return_type)
        : arg_types(std::move(arg_types)), return_type(std::move(return_type))
    {
    }

    void accept(Visitor &visitor) const override
    {
        visitor.visit(*this);
    }
};

using FunctionTypePtr = std::unique_ptr<FunctionType>;

// visitors used for comparing types

struct EquationVisitor;

struct SimpleTypeVisitor : public Visitor
{
    bool value;

    SimpleTypeVisitor(const SimpleType &type) : type(type)
    {
    }

    void visit(const SimpleType &other) override
    {
        this->value = this->type.type == other.type;
    }

    void visit(const NeverType &other) override
    {
        this->value = false;
    }

    void visit(const FunctionType &other) override
    {
        this->value = false;
    }

  private:
    const SimpleType &type;
};

struct NeverTypeVisitor : public Visitor
{
    bool value;

    void visit(const SimpleType &other) override
    {
        this->value = false;
    }

    void visit(const NeverType &other) override
    {
        this->value = true;
    }

    void visit(const FunctionType &other) override
    {
        this->value = false;
    }
};

struct FunctionTypeVisitor : public Visitor
{
    bool value;

    FunctionTypeVisitor(const FunctionType &type) : type(type)
    {
    }

    void visit(const SimpleType &other) override
    {
        this->value = false;
    }

    void visit(const NeverType &other) override
    {
        this->value = false;
    }

    void visit(const FunctionType &other) override
    {
        this->value = (this->type.arg_types == other.arg_types &&
                       this->type.return_type == other.return_type);
    }

  private:
    const FunctionType &type;
};

struct EquationVisitor : public Visitor
{

    bool value;

    EquationVisitor(const Type &type) : type(type)
    {
    }

    void visit(const SimpleType &other) override
    {
        auto visitor = SimpleTypeVisitor(other);
        type.accept(visitor);
        this->value = visitor.value;
    }

    void visit(const NeverType &other) override
    {
        auto visitor = NeverTypeVisitor();
        type.accept(visitor);
        this->value = visitor.value;
    }

    void visit(const FunctionType &other) override
    {
        auto visitor = FunctionTypeVisitor(other);
        type.accept(visitor);
        this->value = visitor.value;
    }

  private:
    const Type &type;
};

inline bool operator==(const Type &first, const Type &other)
{
    auto visitor = EquationVisitor(first);
    other.accept(visitor);
    return visitor.value;
}

#endif