#ifndef __PRINT_VISITOR_HPP__
#define __PRINT_VISITOR_HPP__
#include "ast.hpp"
#include "nlohmann/json.hpp"
#include "visitor.hpp"
#include <map>
#include <ostream>

class PrintVisitor : public AstVisitor
{
    nlohmann::json get_position(const Position &position);

  public:
    // PrintVisitor() : last_object(nullptr)
    // {
    // }
    nlohmann::json last_object;
    PrintVisitor() = default;

    void visit(const VariableExpr &node) override;
    void visit(const I32Expr &node) override;
    void visit(const F64Expr &node) override;
    void visit(const StringExpr &node) override;
    void visit(const CharExpr &node) override;
    void visit(const BoolExpr &node) override;
    void visit(const BinaryExpr &node) override;
    void visit(const UnaryExpr &node) override;
    void visit(const CallExpr &node) override;
    void visit(const LambdaCallExpr &node) override;
    void visit(const IndexExpr &node) override;
    void visit(const CastExpr &node) override;

    void visit(const Block &node) override;
    void visit(const IfStmt &node) override;
    void visit(const WhileStmt &node) override;
    void visit(const MatchStmt &node) override;
    void visit(const ReturnStmt &node) override;
    void visit(const BreakStmt &node) override;
    void visit(const ContinueStmt &node) override;
    void visit(const FuncDefStmt &node) override;
    void visit(const AssignStmt &node) override;
    void visit(const ExprStmt &node) override;
    void visit(const VarDeclStmt &node) override;
    void visit(const ExternStmt &node) override;

    void visit(const LiteralArm &node) override;
    void visit(const GuardArm &node) override;
    void visit(const PlaceholderArm &node) override;

    void visit(const NeverType &type) override;
    void visit(const SimpleType &type) override;
    void visit(const FunctionType &type) override;

    void visit(const Program &node) override;
    void visit(const Parameter &node) override;
};

class JsonSerializer
{
    PrintVisitor visitor;

  public:
    JsonSerializer() = default;
    nlohmann::json serialize(const Program &program);
};
#endif