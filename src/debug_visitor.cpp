#include "debug_visitor.hpp"

void DebugVisitor::visit(const VariableExpr &node)
{
    this->out << node.name;
}

void DebugVisitor::visit(const I32Expr &node)
{
    this->out << node.value;
}

void DebugVisitor::visit(const F32Expr &node)
{
    this->out << node.value << "f";
}

void DebugVisitor::visit(const F64Expr &node)
{
    this->out << node.value << "d";
}

void DebugVisitor::visit(const BinaryExpr &node)
{
    this->out << "(";
    node.lhs->accept(*this);
    node.op->accept(*this);
    node.rhs->accept(*this);
    this->out << ")";
}

void DebugVisitor::visit(const UnaryExpr &node)
{
}

void DebugVisitor::visit(const CallExpr &node)
{
    this->out << node.func_name << "(";
    for (unsigned long i = 0; i < node.args.size(); ++i)
    {
        node.args[i]->accept(*this);
        if (i < node.args.size() - 1)
            this->out << ",";
    }
    this->out << ")";
}

void DebugVisitor::print_optional_args(
    const std::vector<std::optional<ExprNodePtr>> &args, bool &print_ellipsis)
{
    for (unsigned i = 0; i < args.size(); ++i)
    {
        auto &arg = args[i];
        if (arg.has_value())
        {
            arg.value()->accept(*this);
        }
        else
        {
            print_ellipsis = false;
            this->out << "_";
        }
        if (i < args.size() - 1)
            this->out << ",";
    }
}

void DebugVisitor::print_ellipsis_and_commas(const LambdaCallExpr &node,
                                             const bool &print_ellipsis)
{
    if (node.args.size() && (print_ellipsis || node.post_ellipsis_args.size()))
        this->out << ",";
    if ((node.args.size() && print_ellipsis) || node.post_ellipsis_args.size())
        this->out << "...";
    if (node.post_ellipsis_args.size())
        this->out << ",";
}

void DebugVisitor::visit(const LambdaCallExpr &node)
{
    bool print_ellipsis = true;
    this->out << node.func_name << "(";
    this->print_optional_args(node.args, print_ellipsis);
    this->print_ellipsis_and_commas(node, print_ellipsis);
    this->print_optional_args(node.post_ellipsis_args, print_ellipsis);
    this->out << ")";
}

void DebugVisitor::visit(const Block &node)
{
    this->out << "{";
    for (auto &stmt : node.statements)
        stmt->accept(*this);
    this->out << "}";
}

void DebugVisitor::visit(const ReturnStmt &node)
{
    this->out << "ret ";
    node.expr->accept(*this);
    this->out << ";";
}

void DebugVisitor::visit(const FuncDefStmt &node)
{
    this->out << "fn " << node.name << "(";
    for (unsigned long i = 0; i < node.params.size(); ++i)
    {
        this->out << node.params[i]->name << ":";
        node.params[i]->type->accept(*this);
        if (i < node.params.size() - 1)
            this->out << ",";
    }
    this->out << ")=>";
    node.return_type->accept(*this);
    node.block->accept(*this);
}

void DebugVisitor::visit(const AssignStmt &node)
{
}

void DebugVisitor::visit(const VarDeclStmt &node)
{
    this->out << "let " << node.name;
    if (node.type.has_value())
    {
        this->out << ":";
        node.type.value()->accept(*this);
    }
    if (node.initial_value.has_value())
    {
        this->out << "=";
        node.initial_value.value()->accept(*this);
    }
    this->out << ";";
}

void DebugVisitor::visit(const ExternStmt &node)
{
    this->out << "ext " << node.name << "(";
    for (unsigned long i = 0; i < node.params.size(); ++i)
    {
        this->out << node.params[i]->name << ":";
        node.params[i]->type->accept(*this);
        if (i < node.params.size() - 1)
            this->out << ",";
    }
    this->out << ")=>";
    node.return_type->accept(*this);
    this->out << ";";
}

void DebugVisitor::visit(const Program &node)
{
    for (auto &ext : node.externs)
        ext->accept(*this);
    for (auto &var : node.globals)
        var->accept(*this);
    for (auto &function : node.functions)
        function->accept(*this);
}

void DebugVisitor::visit(const NeverType &type)
{
    this->out << L"!";
}

void DebugVisitor::visit(const SimpleType &type)
{
    this->out << this->type_string_map.at(type.type);
}

void DebugVisitor::visit(const FunctionType &type)
{
    this->out << "fn(";
    for (unsigned long i = 0; i < type.arg_types.size(); ++i)
    {
        type.arg_types[i]->accept(*this);
        if (i < type.arg_types.size() - 1)
            this->out << ",";
    }
    this->out << ")=>";
    type.return_type->accept(*this);
}

void DebugVisitor::visit(const BuiltInBinOp &op)
{
    this->out << this->bin_op_strings.at(op.op);
}

void DebugVisitor::visit(const BuiltInUnaryOp &op)
{
    this->out << this->unary_op_strings.at(op.op);
}
