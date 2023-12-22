#ifndef __JSON_SERIALIZER_HPP__
#define __JSON_SERIALIZER_HPP__
#include "ast.hpp"
#include "nlohmann/json.hpp"
#include "visitor.hpp"
#include <unordered_map>

class JsonSerializer
{
    class Visitor : public AstVisitor
    {
        static std::unordered_map<BinOpEnum, std::wstring> binop_map;
        static std::unordered_map<UnaryOpEnum, std::wstring> unop_map;
        static std::unordered_map<AssignType, std::wstring> assign_map;
        static std::unordered_map<RefSpecifier, std::wstring> ref_spec_map;
        static std::unordered_map<TypeEnum, std::wstring> type_map;

        nlohmann::json get_position(const Position &position);

        void visit(const VariableExpr &node);
        void visit(const U32Expr &node);
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

        void visit(const Block &node);
        void visit(const IfStmt &node);
        void visit(const WhileStmt &node);
        void visit(const MatchStmt &node);
        void visit(const ReturnStmt &node);
        void visit(const BreakStmt &node);
        void visit(const ContinueStmt &node);
        void visit(const FuncDefStmt &node);
        void visit(const AssignStmt &node);
        void visit(const ExprStmt &node);
        void visit(const VarDeclStmt &node);
        void visit(const ExternStmt &node);

        void visit(const LiteralArm &node);
        void visit(const GuardArm &node);
        void visit(const ElseArm &node);

        void visit(const SimpleType &type);
        void visit(const FunctionType &type);

      public:
        nlohmann::json last_object;
        Visitor() = default;

        void visit(const ExprNode &node) override;

        void visit(const Statement &node) override;
        void visit(const MatchArm &node) override;
        void visit(const Type &type) override;

        void visit(const Program &node) override;
        void visit(const Parameter &node) override;
    } visitor;

  public:
    JsonSerializer() = default;
    nlohmann::json serialize(const Program &program);
};
#endif