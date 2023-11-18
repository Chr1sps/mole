#ifndef __PRINT_VISITOR_HPP__
#define __PRINT_VISITOR_HPP__
#include "ast.hpp"
#include "operators.hpp"
#include "visitor.hpp"
#include <map>
#include <ostream>

class PrintVisitor : public AstVisitor, BinOpVisitor, TypeVisitor
{
    std::wostream &out;

    static std::map<BinOpEnum, std::wstring> bin_op_strings;
    static std::map<UnaryOpEnum, std::wstring> unary_op_strings;
    static std::map<TypeEnum, std::wstring> type_string_map;
    static std::map<AssignType, std::wstring> assign_strings;

    std::wstring space_if_not_debug();
    std::wstring newline_if_not_debug();
    std::wstring indent_if_not_debug();
    std::wstring lparen_if_debug();
    std::wstring rparen_if_debug();

    void print_indent();
    void increment_indent();
    void decrement_indent();

    unsigned indent_level = 0;
    bool function_block_indent = false;
    bool debug_mode;

  public:
    PrintVisitor(std::wostream &out, const bool &debug)
        : out(out), debug_mode(debug)
    {
    }

    PrintVisitor(std::wostream &out) : PrintVisitor(out, false)
    {
    }

    void visit(const VariableExpr &node) override;
    void visit(const I32Expr &node) override;
    void visit(const F64Expr &node) override;
    void visit(const BinaryExpr &node) override;
    void visit(const UnaryExpr &node) override;
    void visit(const CallExpr &node) override;
    void print_optional_args(
        const std::vector<std::optional<ExprNodePtr>> &args);
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

    void turn_on_debug_mode();
    void turn_off_debug_mode();
};
#endif