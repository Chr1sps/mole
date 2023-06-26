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
    this->out << node.func_name << "(";
    for (unsigned long i = 0; i < node.args.size(); ++i)
    {
        node.args[i]->accept(*this);
        if (i < node.args.size() - 1)
        {
            this->out << ", ";
        }
    }
    this->out << ")" << std::endl;
}

void PrintVisitor::visit(const Block &node)
{
    this->out << "{" << std::endl;
    for (auto &stmt : node.statements)
    {
        stmt->accept(*this);
    }
    this->out << "}" << std::endl;
}

void PrintVisitor::visit(const ReturnStmt &node)
{
    this->out << "return ";
    node.expr->accept(*this);
    this->out << ";" << std::endl;
}

void PrintVisitor::visit(const FuncDefStmt &node)
{
    this->out << "fn " << node.name << "(";
    for (unsigned long i = 0; i < node.params.size(); ++i)
    {
        this->out << node.params[i]->name << ": ";
        node.params[i]->type->accept(*this);
        if (i < node.params.size() - 1)
        {
            this->out << ", ";
        }
    }
    this->out << ") => ";
    node.return_type->accept(*this);
    this->out << " ";
    node.block->accept(*this);
}

void PrintVisitor::visit(const AssignStmt &node)
{
}

void PrintVisitor::visit(const VarDeclStmt &node)
{
    this->out << "let " << node.name;
    if (node.type.has_value())
    {
        this->out << " : ";
        node.type.value()->accept(*this);
    }
    if (node.initial_value.has_value())
    {
        this->out << " = ";
        node.initial_value.value()->accept(*this);
    }
    this->out << ";" << std::endl;
}

void PrintVisitor::visit(const ExternStmt &node)
{
    this->out << "extern " << node.name << "(";
    for (unsigned long i = 0; i < node.params.size(); ++i)
    {
        this->out << node.params[i]->name << ": ";
        node.params[i]->type->accept(*this);
        if (i < node.params.size() - 1)
        {
            this->out << ", ";
        }
    }
    this->out << ") => ";
    node.return_type->accept(*this);
    this->out << ";" << std::endl;
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
    this->out << this->bin_op_strings.at(op.op);
}

void PrintVisitor::visit(const BuiltInUnaryOp &op)
{
    this->out << this->unary_op_strings.at(op.op);
}
