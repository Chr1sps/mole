#ifndef __PRINT_VISITOR_HPP__
#define __PRINT_VISITOR_HPP__
#include "ast.hpp"
#include "operators.hpp"
#include "visitor.hpp"
#include <map>
#include <ostream>

class PrintVisitor : public Visitor
{
    std::wostream &out;
    std::map<BinOpEnum, std::wstring> bin_op_strings = {
        {BinOpEnum::ADD, L"+"},     {BinOpEnum::AND, L"&&"},
        {BinOpEnum::BIT_AND, L"&"}, {BinOpEnum::BIT_OR, L"|"},
        {BinOpEnum::BIT_XOR, L"^"}, {BinOpEnum::DIV, L"/"},
        {BinOpEnum::EQ, L"=="},     {BinOpEnum::EXP, L"^^"},
        {BinOpEnum::GE, L">="},     {BinOpEnum::GT, L">"},
        {BinOpEnum::LE, L"<="},     {BinOpEnum::LT, L">"},
        {BinOpEnum::MOD, L"%"},     {BinOpEnum::MUL, L"*"},
        {BinOpEnum::NEQ, L"!="},    {BinOpEnum::OR, L"||"},
        {BinOpEnum::SHL, L"<<"},    {BinOpEnum::SHR, L">>"},
        {BinOpEnum::SUB, L"-"},

    };
    std::map<UnaryOpEnum, std::wstring> unary_op_strings = {
        {UnaryOpEnum::BIT_NEG, L"~"},
        {UnaryOpEnum::DEC, L"--"},
        {UnaryOpEnum::INC, L"++"},
        {UnaryOpEnum::NEG, L"!"},
    };
    std::map<TypeEnum, std::wstring> type_string_map = {
        {TypeEnum::I32, L"i32"},
        {TypeEnum::F32, L"f32"},
        {TypeEnum::F64, L"f64"},
    };
    void print_indent();
    void increment_indent();
    void decrement_indent();
    unsigned indent_level = 0;
    bool function_block_indent = false;

  public:
    PrintVisitor(std::wostream &out) : out(out)
    {
    }

    void visit(const VariableExpr &node) override;
    void visit(const I32Expr &node) override;
    void visit(const F32Expr &node) override;
    void visit(const F64Expr &node) override;
    void visit(const BinaryExpr &node) override;
    void visit(const UnaryExpr &node) override;
    void visit(const CallExpr &node) override;
    void print_optional_args(
        const std::vector<std::optional<ExprNodePtr>> &args,
        bool &print_ellipsis);
    void print_ellipsis_and_commas(const LambdaCallExpr &node,
                                   const bool &print_ellipsis);
    void visit(const LambdaCallExpr &node) override;
    void visit(const Block &node) override;
    void visit(const ReturnStmt &node) override;
    void print_params(const std::vector<ParamPtr> &params);
    void visit(const FuncDefStmt &node) override;
    void visit(const AssignStmt &node) override;
    void visit(const VarDeclStmt &node) override;
    void visit(const ExternStmt &node) override;
    void visit(const Program &node) override;

    void visit(const NeverType &type) override;
    void visit(const SimpleType &type) override;
    void visit(const FunctionType &type) override;

    void visit(const BuiltInBinOp &op) override;
    void visit(const BuiltInUnaryOp &op) override;
};
#endif