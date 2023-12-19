#include "types.hpp"

void SimpleTypeVisitor::visit(const SimpleType &other)
{
    this->value = this->type.type == other.type;
}

void SimpleTypeVisitor::visit(const FunctionType &other)
{
    this->value = false;
}

void FunctionTypeVisitor::visit(const SimpleType &other)
{
    this->value = false;
}

void FunctionTypeVisitor::visit(const FunctionType &other)
{
    auto are_args_equal =
        std::equal(this->type.arg_types.begin(), this->type.arg_types.end(),
                   other.arg_types.begin(),
                   [](TypePtr a, TypePtr b) { return *a == *b; });
    this->value = (are_args_equal &&
                   ((this->type.return_type == nullptr &&
                     other.return_type == nullptr) ||
                    (this->type.return_type && other.return_type &&
                     *(this->type.return_type) == *(other.return_type))) &&
                   this->type.is_const == other.is_const);
}

void TypeEquationVisitor::visit(const SimpleType &other)
{
    auto visitor = SimpleTypeVisitor(other);
    type.accept(visitor);
    this->value = visitor.value;
}

void TypeEquationVisitor::visit(const FunctionType &other)
{
    auto visitor = FunctionTypeVisitor(other);
    type.accept(visitor);
    this->value = visitor.value;
}

bool operator==(const Type &first, const Type &other)
{
    auto visitor = TypeEquationVisitor(first);
    other.accept(visitor);
    return visitor.value;
}