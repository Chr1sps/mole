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

std::unordered_map<BinOpEnum, std::wstring>
    JsonSerializer::Visitor::binop_map = {
        {BinOpEnum::ADD, L"ADD"},         {BinOpEnum::AND, L"AND"},
        {BinOpEnum::BIT_AND, L"BIT_AND"}, {BinOpEnum::BIT_OR, L"BIT_OR"},
        {BinOpEnum::BIT_XOR, L"BIT_XOR"}, {BinOpEnum::DIV, L"DIV"},
        {BinOpEnum::EQ, L"EQ"},           {BinOpEnum::EXP, L"EXP"},
        {BinOpEnum::GE, L"GE"},           {BinOpEnum::GT, L"GT"},
        {BinOpEnum::LE, L"LE"},           {BinOpEnum::LT, L"LT"},
        {BinOpEnum::MOD, L"MOD"},         {BinOpEnum::MUL, L"MUL"},
        {BinOpEnum::NEQ, L"NEQ"},         {BinOpEnum::OR, L"OR"},
        {BinOpEnum::SHL, L"SHL"},         {BinOpEnum::SHR, L"SHR"},
        {BinOpEnum::SUB, L"SUB"},

};

std::unordered_map<UnaryOpEnum, std::wstring>
    JsonSerializer::Visitor::unop_map = {
        {UnaryOpEnum::BIT_NEG, L"BIT_NEG"}, {UnaryOpEnum::NEG, L"NEG"},
        {UnaryOpEnum::REF, L"REF"},         {UnaryOpEnum::MINUS, L"MINUS"},
        {UnaryOpEnum::MUT_REF, L"MUT_REF"}, {UnaryOpEnum::DEREF, L"DEREF"},
};

std::unordered_map<RefSpecifier, std::wstring>
    JsonSerializer::Visitor::ref_spec_map = {
        {RefSpecifier::NON_REF, L"NON_REF"},
        {RefSpecifier::REF, L"REF"},
        {RefSpecifier::MUT_REF, L"MUT_REF"},
};
std::unordered_map<AssignType, std::wstring>
    JsonSerializer::Visitor::assign_map = {
        {AssignType::NORMAL, L"NORMAL"}, {AssignType::PLUS, L"PLUS"},
        {AssignType::MINUS, L"MINUS"},   {AssignType::MUL, L"MUL"},
        {AssignType::DIV, L"DIV"},       {AssignType::MOD, L"MOD"},
        {AssignType::EXP, L"EXP"},       {AssignType::BIT_AND, L"BIT_AND"},
        {AssignType::BIT_OR, L"BIT_OR"}, {AssignType::BIT_XOR, L"BIT_XOR"},
        {AssignType::SHR, L"SHR"},       {AssignType::SHL, L"SHL"},
};
std::unordered_map<TypeEnum, std::wstring> JsonSerializer::Visitor::type_map =
    {
        {TypeEnum::BOOL, L"BOOL"}, {TypeEnum::U32, L"U32"},
        {TypeEnum::I32, L"I32"},   {TypeEnum::F64, L"F64"},
        {TypeEnum::CHAR, L"CHAR"}, {TypeEnum::STR, L"STR"},
};

nlohmann::json JsonSerializer::Visitor::get_position(const Position &position)
{
    nlohmann::json result;
    result["line"] = position.line;
    result["column"] = position.column;
    return result;
}

void JsonSerializer::Visitor::visit(const VariableExpr &node)
{
    nlohmann::json output;
    output["type"] = "VarExpr";
    output["value"] = node.name;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const U32Expr &node)
{
    nlohmann::json output;
    output["type"] = "U32Expr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const F64Expr &node)
{
    nlohmann::json output;
    output["type"] = "F64Expr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const StringExpr &node)
{
    nlohmann::json output;
    output["type"] = "StringExpr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const CharExpr &node)
{
    nlohmann::json output;
    output["type"] = "CharExpr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const BoolExpr &node)
{
    nlohmann::json output;
    output["type"] = "BoolExpr";
    output["value"] = node.value;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const BinaryExpr &node)
{
    nlohmann::json output;
    output["type"] = "BinaryExpr";
    this->visit(*node.lhs);
    output["lhs"] = this->last_object;
    output["op"] = this->binop_map.at(node.op);
    this->visit(*node.rhs);
    output["rhs"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const UnaryExpr &node)
{
    nlohmann::json output;
    output["type"] = "UnaryExpr";
    output["op"] = this->unop_map.at(node.op);
    this->visit(*node.expr);
    output["expr"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit_call(const CallExpr &node)
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

void JsonSerializer::Visitor::visit(const IndexExpr &node)
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

void JsonSerializer::Visitor::visit(const CastExpr &node)
{
    nlohmann::json output;
    output["type"] = "CastExpr";
    this->visit(*node.expr);
    output["expr"] = this->last_object;
    this->visit(node.type);
    output["cast_type"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const Expression &node)
{
    // this looks like a cheap, half-assed version of double-dispatch, but
    // whatever
    std::visit(
        overloaded{[this](const VariableExpr &node) { this->visit(node); },
                   [this](const U32Expr &node) { this->visit(node); },
                   [this](const F64Expr &node) { this->visit(node); },
                   [this](const StringExpr &node) { this->visit(node); },
                   [this](const CharExpr &node) { this->visit(node); },
                   [this](const BoolExpr &node) { this->visit(node); },
                   [this](const BinaryExpr &node) { this->visit(node); },
                   [this](const UnaryExpr &node) { this->visit(node); },
                   [this](const CallExpr &node) { this->visit_call(node); },
                   [this](const IndexExpr &node) { this->visit(node); },
                   [this](const CastExpr &node) { this->visit(node); }},
        node);
}

void JsonSerializer::Visitor::visit_block(const Block &node)
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

void JsonSerializer::Visitor::visit(const IfStmt &node)
{
    nlohmann::json output;
    output["type"] = "IfStmt";
    this->visit(*node.condition_expr);
    output["condition"] = this->last_object;
    this->visit(*node.then_block);
    output["then_block"] = this->last_object;
    if (node.else_block)
    {
        this->visit(*node.else_block);
        output["else_block"] = this->last_object;
    }
    else
        output["else_block"] = nullptr;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const WhileStmt &node)
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

void JsonSerializer::Visitor::visit(const MatchStmt &node)
{
    nlohmann::json output;
    output["type"] = "MatchStmt";
    this->visit(*node.matched_expr);
    output["matched_expr"] = this->last_object;
    output["arms"] = nlohmann::json::array();
    for (const auto &arm : node.match_arms)
    {
        this->visit(*arm);
        output["arms"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const ReturnStmt &node)
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

void JsonSerializer::Visitor::visit(const BreakStmt &node)
{
    nlohmann::json output;
    output["type"] = "BreakStmt";
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const ContinueStmt &node)
{
    nlohmann::json output;
    output["type"] = "ContinueStmt";
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const FuncDef &node)
{
    nlohmann::json output;
    output["type"] = "FuncDef";
    output["name"] = node.name;
    output["const"] = node.is_const;
    output["params"] = nlohmann::json::array();
    for (const auto &param : node.params)
    {
        this->visit(*param);
        output["params"] = this->last_object;
    }
    if (node.return_type)
    {
        this->visit(*node.return_type);
        output["return_type"] = this->last_object;
    }
    this->visit_block(*node.block);
    output["block"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const AssignStmt &node)
{
    nlohmann::json output;
    output["type"] = "AssignStmt";
    this->visit(*node.lhs);
    output["lhs"] = this->last_object;
    output["op"] = this->assign_map.at(node.type);
    this->visit(*node.rhs);
    output["rhs"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const ExprStmt &node)
{
    nlohmann::json output;
    output["type"] = "AssignStmt";
    this->visit(*node.expr);
    output["expr"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const VarDeclStmt &node)
{
    nlohmann::json output;
    output["type"] = "VarDeclStmt";
    output["name"] = node.name;
    if (node.type)
    {
        this->visit(*node.type);
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

void JsonSerializer::Visitor::visit(const ExternDef &node)
{
    nlohmann::json output;
    output["type"] = "ExternDef";
    output["name"] = node.name;
    for (const auto &param : node.params)
    {
        this->visit(*param);
    }
    if (node.return_type)
    {
        this->visit(*node.return_type);
        output["return_type"] = this->last_object;
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const Statement &node)
{
    std::visit(
        overloaded{[this](const Block &node) { this->visit_block(node); },
                   [this](const IfStmt &node) { this->visit(node); },
                   [this](const WhileStmt &node) { this->visit(node); },
                   [this](const MatchStmt &node) { this->visit(node); },
                   [this](const ReturnStmt &node) { this->visit(node); },
                   [this](const BreakStmt &node) { this->visit(node); },
                   [this](const ContinueStmt &node) { this->visit(node); },
                   [this](const AssignStmt &node) { this->visit(node); },
                   [this](const ExprStmt &node) { this->visit(node); },
                   [this](const VarDeclStmt &node) { this->visit(node); }},
        node);
}

void JsonSerializer::Visitor::visit(const LiteralArm &node)
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

void JsonSerializer::Visitor::visit(const GuardArm &node)
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

void JsonSerializer::Visitor::visit(const ElseArm &node)
{
    nlohmann::json output;
    output["type"] = "ElseArm";
    this->visit(*node.block);
    output["block"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const MatchArm &node)
{
    std::visit(
        overloaded{[this](const LiteralArm &node) { this->visit(node); },
                   [this](const GuardArm &node) { this->visit(node); },
                   [this](const ElseArm &node) { this->visit(node); }},
        node);
}

void JsonSerializer::Visitor::visit(const Program &node)
{
    nlohmann::json output;
    output["type"] = "Program";
    output["externs"] = output["globals"] = output["functions"] =
        nlohmann::json::array();
    for (auto &ext : node.externs)
    {
        this->visit(*ext);
        output["externs"].push_back(this->last_object);
    }
    for (auto &var : node.globals)
    {
        this->visit(*var);
        output["globals"].push_back(this->last_object);
    }
    for (auto &function : node.functions)
    {
        this->visit(*function);
        output["functions"].push_back(this->last_object);
    }
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const Parameter &node)
{
    nlohmann::json output;
    output["type"] = "Program";
    output["name"] = node.name;
    this->visit(node.type);
    output["param_type"] = this->last_object;
    output["position"] = this->get_position(node.position);
    this->last_object = output;
}

void JsonSerializer::Visitor::visit(const Type &type)
{
    nlohmann::json output;
    output["type"] = "Type";
    output["ref_spec"] = this->ref_spec_map.at(type.ref_spec);
    output["value"] = this->type_map.at(type.type);
    this->last_object = output;
}

nlohmann::json JsonSerializer::serialize(const Program &program)
{
    this->visitor.visit(program);
    return this->visitor.last_object;
}
