#include "print_visitor.hpp"

std::wstring PrintVisitor::repr_operator(const BuiltInBinOp &op)
{
    return this->bin_op_strings.at(op.op);
}

std::wstring PrintVisitor::repr_operator(const BuiltInUnaryOp &op)
{
    return this->unary_op_strings.at(op.op);
}

void PrintVisitor::visit(const VariableExpr &node)
{
    this->out << node.name;
}

void PrintVisitor::visit(const I32Expr &node)
{
    this->out << node.value;
}

void PrintVisitor::visit(const F32Expr &node)
{
    this->out << node.value;
}

void PrintVisitor::visit(const F64Expr &node)
{
    this->out << node.value;
}

void PrintVisitor::visit(const BinaryExpr &node)
{
    node.lhs->accept(*this);
    node.op->accept(*this);
    node.rhs->accept(*this);
}

void PrintVisitor::visit(const UnaryExpr &node)
{
}

void PrintVisitor::visit(const CallExpr &node)
{
}

void PrintVisitor::visit(const Block &node)
{
}

void PrintVisitor::visit(const ReturnStmt &node)
{
}

void PrintVisitor::visit(const FuncDefStmt &node)
{
}

void PrintVisitor::visit(const AssignStmt &node)
{
}

void PrintVisitor::visit(const VarDeclStmt &node)
{
}

void PrintVisitor::visit(const ExternStmt &node)
{
    // this->out << "extern " << node.name << "(";
    // for (int i = 0; i < node.params.size(); ++i)
    // {
    //     auto name = node.params[i]->name;
    //     auto type = node.params[i]->type;
    //     if (i < node.params.size() - 1)
    //     {
    //         this->out << ", ";
    //     }
    // }
}

void PrintVisitor::visit(const Program &node)
{
    for (auto &ext : node.externs)
    {
        ext->accept(*this);
    }
    for (auto &var : node.globals)
    {
        var->accept(*this);
    }
    for (auto &function : node.functions)
    {
        function->accept(*this);
    }
}

void PrintVisitor::visit(const NeverType &type)
{
    this->out << L"!";
}

void PrintVisitor::visit(const SimpleType &type)
{
    this->out << this->type_string_map.at(type.type);
}

void PrintVisitor::visit(const FunctionType &type)
{
    this->out << "fn(";
    for (unsigned long i = 0; i < type.arg_types.size(); ++i)
    {
        type.arg_types[i]->accept(*this);
        if (i < type.arg_types.size() - 1)
            this->out << ", ";
    }
    this->out << ") => ";
    type.return_type->accept(*this);
}

void PrintVisitor::visit(const BuiltInBinOp &op)
{
}

void PrintVisitor::visit(const BuiltInUnaryOp &op)
{
}
