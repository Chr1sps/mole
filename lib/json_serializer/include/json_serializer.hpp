#ifndef __JSON_SERIALIZER_HPP__
#define __JSON_SERIALIZER_HPP__
#include "ast.hpp"
#include "nlohmann/json.hpp"
#include "visitor.hpp"
#include <map>
#include <ostream>

class JsonSerializer
{
    class JsonVisitor : public AstVisitor
    {
        nlohmann::json get_position(const Position &position);

      public:
        nlohmann::json last_object;
        JsonVisitor() = default;

        void visit(const VariableExpr &node);
        void visit(const I32Expr &node);
        void visit(const F64Expr &node);
        void visit(const StringExpr &node);
        void visit(const CharExpr &node);
        void visit(const BoolExpr &node);
        void visit(const BinaryExpr &node);
        void visit(const UnaryExpr &node);
        void visit(const CallExpr &node);
        void visit(const LambdaCallExpr &node);
        void visit(const IndexExpr &node);
        void visit(const CastExpr &node);

        void visit(const ExprNode &node);

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
        void visit(const ElseArm &node) override;

        void visit(const SimpleType &type) override;
        void visit(const FunctionType &type) override;

        void visit(const Program &node) override;
        void visit(const Parameter &node) override;
    } visitor;

  public:
    JsonSerializer() = default;
    nlohmann::json serialize(const Program &program);
};
#endif