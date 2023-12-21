#include "json_serializer.hpp"
#include "nlohmann/json.hpp"
#include <functional>

// Because std::wstring serialization creates an array of integers by
// default, a custom serialization definition is needed.
namespace nlohmann
{
template <> struct adl_serializer<std::wstring>
{
    static void to_json(json &j, const std::wstring &str)
    {
        j = std::string(str.begin(), str.end());
    }
};
} // namespace nlohmann

nlohmann::json JsonSerializer::JsonVisitor::get_position(
    const Position &position)
{
    nlohmann::json result;
    result["line"] = position.line;
    result["column"] = position.column;
    return result;
}

void JsonSerializer::JsonVisitor::visit(const VariableExpr &node)
{
    nlohmann::json output;
    output["type"] = "VarExpr";
    output["value"] = node.name;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const I32Expr &node)
{
    nlohmann::json output;
    output["type"] = "I32Expr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const F64Expr &node)
{
    nlohmann::json output;
    output["type"] = "F64Expr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const StringExpr &node)
{
    nlohmann::json output;
    output["type"] = "StringExpr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const CharExpr &node)
{
    nlohmann::json output;
    output["type"] = "CharExpr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const BoolExpr &node)
{
    nlohmann::json output;
    output["type"] = "BoolExpr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const BinaryExpr &node)
{
    nlohmann::json output;
    output["type"] = "BinaryExpr";
    this->visit(*node.lhs);
    output["lhs"] = this->last_object;
    // output["op"] = ;
    this->visit(*node.rhs);
    output["rhs"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const UnaryExpr &node)
{
    nlohmann::json output;
    output["type"] = "UnaryExpr";
    // output["op"] = ;
    this->visit(*node.expr);
    output["expr"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const CallExpr &node)
{
    nlohmann::json output;
    output["type"] = "CallExpr";
    output["args"] = nlohmann::json::array();
    for (const auto &arg : node.args)
    {
        this->visit(*arg);
        output["args"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const LambdaCallExpr &node)
{
    nlohmann::json output;
    output["type"] = "LambdaCallExpr";
    output["args"] = nlohmann::json::array();
    for (const auto &arg : node.args)
    {
        if (arg)
        {
            this->visit(*arg);
            output["args"].push_back(this->last_object);
        }
        else
            output["args"].push_back(nullptr);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const IndexExpr &node)
{
    nlohmann::json output;
    output["type"] = "IndexExpr";
    this->visit(*node.expr);
    output["expr"] = this->last_object;
    this->visit(*node.index_value);
    output["index_value"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const CastExpr &node)
{
    nlohmann::json output;
    output["type"] = "CastExpr";
    this->visit(*node.expr);
    output["expr"] = this->last_object;
    node.type->accept(*this);
    output["cast_type"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const ExprNode &node)
{
    // this looks like a cheap, half-assed version of double-dispatch, but
    // whatever
    std::visit(
        overloaded{[this](const VariableExpr &node) { this->visit(node); },
                   [this](const I32Expr &node) { this->visit(node); },
                   [this](const F64Expr &node) { this->visit(node); },
                   [this](const StringExpr &node) { this->visit(node); },
                   [this](const CharExpr &node) { this->visit(node); },
                   [this](const BoolExpr &node) { this->visit(node); },
                   [this](const BinaryExpr &node) { this->visit(node); },
                   [this](const UnaryExpr &node) { this->visit(node); },
                   [this](const CallExpr &node) { this->visit(node); },
                   [this](const LambdaCallExpr &node) { this->visit(node); },
                   [this](const IndexExpr &node) { this->visit(node); },
                   [this](const CastExpr &node) { this->visit(node); }},
        node);
}

void JsonSerializer::JsonVisitor::visit(const Block &node)
{
    nlohmann::json output;
    output["type"] = "Block";
    output["stmts"] = nlohmann::json::array();
    for (const auto &stmt : node.statements)
    {
        this->visit(*stmt);
        output["stmts"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const IfStmt &node)
{
    nlohmann::json output;
    output["type"] = "IfStmt";
    this->visit(*node.condition_expr);
    output["condition"] = this->last_object;
    this->visit(*node.then_block);
    output["then_block"] = this->last_object;
    this->visit(*node.else_block);
    output["else_block"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const WhileStmt &node)
{
    nlohmann::json output;
    output["type"] = "IfStmt";
    this->visit(*node.condition_expr);
    output["condition"] = this->last_object;
    this->visit(*node.statement);
    output["statement"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const MatchStmt &node)
{
    nlohmann::json output;
    output["type"] = "MatchStmt";
    this->visit(*node.matched_expr);
    output["matched_expr"] = this->last_object;
    output["arms"] = nlohmann::json::array();
    for (const auto &arm : node.match_arms)
    {
        arm->accept(*this);
        output["arms"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const ReturnStmt &node)
{
    nlohmann::json output;
    output["type"] = "ReturnStmt";
    if (node.expr)
    {
        this->visit(*node.expr);
        output["value"] = this->last_object;
    }
    else
    {
        output["value"] = nullptr;
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const BreakStmt &node)
{
    nlohmann::json output;
    output["type"] = "BreakStmt";
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const ContinueStmt &node)
{
    nlohmann::json output;
    output["type"] = "ContinueStmt";
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const FuncDefStmt &node)
{
    nlohmann::json output;
    output["type"] = "FuncDefStmt";
    output["name"] = node.name;
    output["const"] = node.is_const;
    output["params"] = nlohmann::json::array();
    for (const auto &param : node.params)
    {
        param->accept(*this);
        output["params"] = this->last_object;
    }
    if (node.return_type)
    {
        node.return_type->accept(*this);
        output["return_type"] = this->last_object;
    }
    this->visit(*node.block);
    output["block"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const AssignStmt &node)
{
    nlohmann::json output;
    output["type"] = "AssignStmt";
    this->visit(*node.lhs);
    output["lhs"] = this->last_object;
    // output["op"] = node.type
    output["op"] = "TODO: OP TYPE";
    this->visit(*node.rhs);
    output["rhs"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const ExprStmt &node)
{
    nlohmann::json output;
    output["type"] = "AssignStmt";
    this->visit(*node.expr);
    output["expr"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const VarDeclStmt &node)
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
        this->visit(*node.initial_value);
        output["value"] = this->last_object;
    }
    else
    {
        output["value"] = nullptr;
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const ExternStmt &node)
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

void JsonSerializer::JsonVisitor::visit(const Statement &node)
{

    std::visit(
        overloaded{[this](const Block &node) { this->visit(node); },
                   [this](const IfStmt &node) { this->visit(node); },
                   [this](const WhileStmt &node) { this->visit(node); },
                   [this](const MatchStmt &node) { this->visit(node); },
                   [this](const ReturnStmt &node) { this->visit(node); },
                   [this](const BreakStmt &node) { this->visit(node); },
                   [this](const ContinueStmt &node) { this->visit(node); },
                   [this](const FuncDefStmt &node) { this->visit(node); },
                   [this](const AssignStmt &node) { this->visit(node); },
                   [this](const ExprStmt &node) { this->visit(node); },
                   [this](const VarDeclStmt &node) { this->visit(node); },
                   [this](const ExternStmt &node) { this->visit(node); }},
        node);
}

void JsonSerializer::JsonVisitor::visit(const Program &node)
{
    nlohmann::json output;
    output["type"] = "Program";
    output["externs"] = nlohmann::json::array();
    for (auto &ext : node.externs)
    {
        this->visit(*ext);
        output["externs"].push_back(this->last_object);
    }
    output["globals"] = nlohmann::json::array();
    for (auto &var : node.globals)
    {
        this->visit(*var);
        output["globals"].push_back(this->last_object);
    }
    output["functions"] = nlohmann::json::array();
    for (auto &function : node.functions)
    {
        this->visit(*function);
        output["functions"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const Parameter &node)
{
    nlohmann::json output;
    output["type"] = "Program";
    output["name"] = node.name;
    node.type->accept(*this);
    output["param_type"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const LiteralArm &node)
{
    nlohmann::json output;
    output["type"] = "LiteralArm";
    output["literals"] = nlohmann::json::array();
    for (const auto &literal : node.literals)
    {
        this->visit(*literal);
        output["literals"].push_back(this->last_object);
    }
    this->visit(*node.block);
    output["block"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const GuardArm &node)
{
    nlohmann::json output;
    output["type"] = "GuardArm";
    this->visit(*node.condition_expr);
    output["condition"] = this->last_object;
    this->visit(*node.block);
    output["block"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const ElseArm &node)
{
    nlohmann::json output;
    output["type"] = "ElseArm";
    this->visit(*node.block);
    output["block"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const SimpleType &type)
{
    nlohmann::json output;
    output["type"] = "SimpleType";
    output["ref_spec"] = "TODO";
    // output["value"] = type.type;
    output["value"] = "TODO";
    // output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::JsonVisitor::visit(const FunctionType &type)
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
