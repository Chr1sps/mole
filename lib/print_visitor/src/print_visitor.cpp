#include "print_visitor.hpp"
#include "nlohmann/json.hpp"
#include <functional>

nlohmann::json PrintVisitor::get_position(const Position &position)
{
    nlohmann::json result;
    result["line"] = position.line;
    result["column"] = position.column;
    return result;
}

void PrintVisitor::visit(const VariableExpr &node)
{
    nlohmann::json output;
    output["type"] = "VarExpr";
    output["value"] = node.name;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const I32Expr &node)
{
    nlohmann::json output;
    output["type"] = "I32Expr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const F64Expr &node)
{
    nlohmann::json output;
    output["type"] = "F64Expr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const BinaryExpr &node)
{
    nlohmann::json output;
    output["type"] = "BinaryExpr";
    node.lhs->accept(*this);
    output["lhs"] = this->last_object;
    // output["op"] = ;
    node.rhs->accept(*this);
    output["rhs"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const UnaryExpr &node)
{
    nlohmann::json output;
    output["type"] = "UnaryExpr";
    // output["op"] = ;
    node.expr->accept(*this);
    output["expr"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const CallExpr &node)
{
    nlohmann::json output;
    output["type"] = "CallExpr";
    for (const auto &arg : node.args)
    {
        arg->accept(*this);
        output["args"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const LambdaCallExpr &node)
{
    nlohmann::json output;
    output["type"] = "LambdaCallExpr";
    for (const auto &arg : node.args)
    {
        if (arg)
        {
            arg->accept(*this);
            output["args"].push_back(this->last_object);
        }
        else
            output["args"].push_back(nullptr);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const Block &node)
{
    nlohmann::json output;
    output["type"] = "Block";
    for (const auto &stmt : node.statements)
    {
        stmt->accept(*this);
        output["stmts"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const ReturnStmt &node)
{
    nlohmann::json output;
    output["type"] = "ReturnStmt";
    if (node.expr)
    {
        node.expr->accept(*this);
        output["value"] = this->last_object;
    }
    else
    {
        output["value"] = nullptr;
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const FuncDefStmt &node)
{
    nlohmann::json output;
    output["type"] = "FuncDefStmt";
    output["name"] = node.name;
    output["const"] = node.is_const;
    for (const auto &param : node.params)
    {
        param->accept(*this);
    }
    if (node.return_type)
    {
        node.return_type->accept(*this);
        output["return_type"] = this->last_object;
    }
    node.block->accept(*this);
    output["block"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const AssignStmt &node)
{
    nlohmann::json output;
    output["type"] = "AssignStmt";
    node.lhs->accept(*this);
    output["lhs"] = this->last_object;
    // output["op"] = node.type
    node.rhs->accept(*this);
    output["rhs"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const VarDeclStmt &node)
{
    nlohmann::json output;
    output["type"] = "VarDeclStmt";
    output["name"] = node.name;
    if (node.type)
    {
        node.type->accept(*this);
        output["var_type"] = this->last_object;
    }
    else
    {
        output["var_type"] = nullptr;
    }
    output["mut"] = node.is_mut;
    if (node.initial_value)
    {
        node.initial_value->accept(*this);
        output["value"] = this->last_object;
    }
    else
    {
        output["value"] = nullptr;
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const ExternStmt &node)
{
    nlohmann::json output;
    output["type"] = "ExternStmt";
    output["name"] = node.name;
    for (const auto &param : node.params)
    {
        param->accept(*this);
    }
    if (node.return_type)
    {
        node.return_type->accept(*this);
        output["return_type"] = this->last_object;
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const Program &node)
{
    nlohmann::json output;
    output["type"] = "Program";
    for (auto &ext : node.externs)
    {
        ext->accept(*this);
        output["externs"].push_back(this->last_object);
    }
    for (auto &var : node.globals)
    {
        var->accept(*this);
        output["globals"].push_back(this->last_object);
    }
    for (auto &function : node.functions)
    {
        function->accept(*this);
        output["functions"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const SimpleType &type)
{
    nlohmann::json output;
    output["type"] = "SimpleType";
    output["ref_spec"] = "TODO";
    // output["value"] = type.type;
    output["value"] = "TODO";
    // output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void PrintVisitor::visit(const FunctionType &type)
{
    nlohmann::json output;
    output["type"] = "FnType";
    output["const"] = type.is_const;
    for (const auto &arg_type : type.arg_types)
    {
        arg_type->accept(*this);
        output["args"] = this->last_object;
    }
    if (type.return_type)
    {
        type.return_type->accept(*this);
        output["return_type"] = this->last_object;
    }
    else
    {
        output["return_type"] = nullptr;
    }
    this->last_object = output;
}

nlohmann::json JsonSerializer::serialize(const Program &program)
{
    program.accept(this->visitor);
    return this->visitor.last_object;
}