#include "types.hpp"

void SimpleTypeVisitor::visit(const SimpleType &other)
{
    this->value = this->type.type == other.type;
}

void SimpleTypeVisitor::visit(const NeverType &other)
{
    this->value = false;
}

void SimpleTypeVisitor::visit(const FunctionType &other)
{
    this->value = false;
}

void NeverTypeVisitor::visit(const SimpleType &other)
{
    this->value = false;
}

void NeverTypeVisitor::visit(const NeverType &other)
{
    this->value = true;
}

void NeverTypeVisitor::visit(const FunctionType &other)
{
    this->value = false;
}

void FunctionTypeVisitor::visit(const SimpleType &other)
{
    this->value = false;
}

void FunctionTypeVisitor::visit(const NeverType &other)
{
    this->value = false;
}

void FunctionTypeVisitor::visit(const FunctionType &other)
{
    this->value = (this->type.arg_types == other.arg_types &&
                   *(this->type.return_type) == *(other.return_type) &&
                   this->type.is_const == other.is_const);
}

void EquationVisitor::visit(const SimpleType &other)
{
    auto visitor = SimpleTypeVisitor(other);
    type.accept(visitor);
    this->value = visitor.value;
}

void EquationVisitor::visit(const NeverType &other)
{
    auto visitor = NeverTypeVisitor();
    type.accept(visitor);
    this->value = visitor.value;
}

void EquationVisitor::visit(const FunctionType &other)
{
    auto visitor = FunctionTypeVisitor(other);
    type.accept(visitor);
    this->value = visitor.value;
}

bool operator==(const Type &first, const Type &other)
{
    auto visitor = EquationVisitor(first);
    other.accept(visitor);
    return visitor.value;
}