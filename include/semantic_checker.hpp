#ifndef __SEMANTIC_CHECKER_HPP__
#define __SEMANTIC_CHECKER_HPP__
#include "types.hpp"
#include "visitor.hpp"
#include <optional>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>

class SemanticChecker : public AstVisitor
{
    struct Variable
    {
        std::wstring name;
        TypePtr &type;
        bool mut;
    };

    struct Function
    {
        std::wstring name;
        std::shared_ptr<FunctionType> &type;
    };

    std::vector<std::set<Variable>> variables;
    std::vector<std::set<Function>> functions;
    std::stack<std::optional<TypePtr>> return_stack;

    void enter_scope();
    void leave_scope();
    void register_globals(const Program &node);

  public:
    void visit(const VariableExpr &node) override;
    void visit(const I32Expr &node) override;
    void visit(const F64Expr &node) override;
    void visit(const BinaryExpr &node) override;
    void visit(const UnaryExpr &node) override;
    void visit(const CallExpr &node) override;
    void visit(const LambdaCallExpr &node) override;
    void visit(const Block &node) override;
    void visit(const ReturnStmt &node) override;
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