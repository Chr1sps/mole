#include "print_visitor.hpp"
#include <functional>

void PrintVisitor::print_indent()
{
    this->out << this->indent_if_not_debug();
}

void PrintVisitor::increment_indent()
{
    ++(this->indent_level);
}

void PrintVisitor::decrement_indent()
{
    --(this->indent_level);
}

std::wstring PrintVisitor::space_if_not_debug()
{
    return (this->debug_mode) ? (L"") : (L" ");
}

std::wstring PrintVisitor::newline_if_not_debug()
{
    return (this->debug_mode) ? (L"") : (L"\n");
}

std::wstring PrintVisitor::indent_if_not_debug()
{
    if (this->debug_mode)
        return L"";
    else
    {
        auto result = std::wstring();
        for (unsigned i = 0; i < this->indent_level * 4; ++i)
            result += L" ";
        return result;
    }
}

std::wstring PrintVisitor::lparen_if_debug()
{
    return (this->debug_mode) ? (L"(") : (L"");
}

std::wstring PrintVisitor::rparen_if_debug()
{
    return (this->debug_mode) ? (L")") : (L"");
}

void PrintVisitor::visit(const VariableExpr &node)
{
    this->out << node.name;
}

void PrintVisitor::visit(const I32Expr &node)
{
    this->out << node.value;
}

void PrintVisitor::visit(const F64Expr &node)
{
    this->out << node.value;
}

void PrintVisitor::visit(const BinaryExpr &node)
{
    this->out << this->lparen_if_debug();
    node.lhs->accept(*this);
    this->out << this->space_if_not_debug();
    node.op->accept(*this);
    this->out << this->space_if_not_debug();
    node.rhs->accept(*this);
    this->out << this->rparen_if_debug();
}

void PrintVisitor::visit(const UnaryExpr &node)
{
    this->out << this->lparen_if_debug();
    node.op->accept(*this);
    node.expr->accept(*this);
    this->out << this->rparen_if_debug();
}

void PrintVisitor::visit(const CallExpr &node)
{
    this->out << node.func_name << "(";
    for (unsigned long i = 0; i < node.args.size(); ++i)
    {
        node.args[i]->accept(*this);
        if (i < node.args.size() - 1)
            this->out << "," << this->space_if_not_debug();
    }
    this->out << ")";
}

void PrintVisitor::print_optional_args(
    const std::vector<std::optional<ExprNodePtr>> &args)
{
    for (unsigned i = 0; i < args.size(); ++i)
    {
        auto &arg = args[i];
        if (arg.has_value())
            arg.value()->accept(*this);
        else
            this->out << "_";
        if (i < args.size() - 1)
            this->out << "," << this->space_if_not_debug();
    }
}

void PrintVisitor::visit(const LambdaCallExpr &node)
{
    this->out << node.func_name << "(";
    this->print_optional_args(node.args);
    if (node.is_ellipsis)
        this->out << "," << this->space_if_not_debug() << "...";
    this->out << ")";
}

void PrintVisitor::visit(const Block &node)
{
    if (this->function_block_indent)
        this->function_block_indent = false;
    else
        this->print_indent();
    this->out << "{" << this->newline_if_not_debug();
    this->increment_indent();
    for (auto &stmt : node.statements)
        stmt->accept(*this);
    this->decrement_indent();
    this->print_indent();
    this->out << "}" << this->newline_if_not_debug();
}

void PrintVisitor::visit(const ReturnStmt &node)
{
    this->print_indent();
    this->out << "return";
    if (node.expr.has_value())
    {
        this->out << " ";
        (*node.expr)->accept(*this);
    }
    this->out << ";" << this->newline_if_not_debug();
}

void PrintVisitor::print_params(const std::vector<ParamPtr> &params)
{
    for (unsigned long i = 0; i < params.size(); ++i)
    {
        this->out << params[i]->name << ":" << this->space_if_not_debug();
        params[i]->type->accept(*this);
        if (i < params.size() - 1)
            this->out << "," << this->space_if_not_debug();
    }
}

void PrintVisitor::visit(const FuncDefStmt &node)
{
    this->print_indent();
    this->out << "fn ";
    if (node.is_const)
        this->out << "const ";
    this->out << node.name << "(";
    this->print_params(node.params);
    this->out << ")" << this->space_if_not_debug() << "=>"
              << this->space_if_not_debug();
    node.return_type->accept(*this);
    this->out << this->space_if_not_debug();
    this->function_block_indent = true;
    node.block->accept(*this);
}

void PrintVisitor::visit(const AssignStmt &node)
{
    this->print_indent();
    this->out << node.name << this->space_if_not_debug()
              << this->assign_strings.at(node.type)
              << this->space_if_not_debug();
    node.value->accept(*this);
    this->out << L";" << this->newline_if_not_debug();
}

void PrintVisitor::visit(const VarDeclStmt &node)
{
    this->print_indent();
    this->out << "let ";
    if (node.is_mut)
        this->out << "mut ";
    this->out << node.name;
    if (node.type.has_value())
    {
        this->out << this->space_if_not_debug() << ":"
                  << this->space_if_not_debug();
        node.type.value()->accept(*this);
    }
    if (node.initial_value.has_value())
    {
        this->out << this->space_if_not_debug() << "="
                  << this->space_if_not_debug();
        node.initial_value.value()->accept(*this);
    }
    this->out << ";" << this->newline_if_not_debug();
}

void PrintVisitor::visit(const ExternStmt &node)
{
    this->print_indent();
    this->out << "extern " << node.name << "(";
    this->print_params(node.params);
    this->out << ")" << this->space_if_not_debug() << "=>"
              << this->space_if_not_debug();
    node.return_type->accept(*this);
    this->out << ";" << this->newline_if_not_debug();
}

void PrintVisitor::visit(const Program &node)
{
    for (auto &ext : node.externs)
        ext->accept(*this);
    for (auto &var : node.globals)
        var->accept(*this);
    for (auto &function : node.functions)
        function->accept(*this);
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
    this->out << "fn";
    if (type.is_const)
        this->out << " const";
    this->out << "(";
    for (unsigned long i = 0; i < type.arg_types.size(); ++i)
    {
        type.arg_types[i]->accept(*this);
        if (i < type.arg_types.size() - 1)
            this->out << ",";
    }
    this->out << ")" << this->space_if_not_debug() << "=>"
              << this->space_if_not_debug();
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

void PrintVisitor::turn_on_debug_mode()
{
    this->debug_mode = true;
}

void PrintVisitor::turn_off_debug_mode()
{
    this->debug_mode = false;
}
