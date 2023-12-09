#ifndef __VISITOR_HPP__
#define __VISITOR_HPP__
#include <memory>
struct VariableExpr;
struct I32Expr;
struct F64Expr;
struct StringExpr;
struct CharExpr;
struct BoolExpr;
struct BinaryExpr;
struct UnaryExpr;
struct CallExpr;
struct LambdaCallExpr;
struct Block;
struct IfStmt;
struct WhileStmt;
struct MatchStmt;
struct LiteralArm;
struct GuardArm;
struct PlaceholderArm;
struct BreakStmt;
struct ContinueStmt;
struct ReturnStmt;
struct FuncDefStmt;
struct AssignStmt;
struct VarDeclStmt;
struct ExternStmt;
struct Program;
struct Parameter;

struct NeverType;
struct SimpleType;
struct FunctionType;

class Visitor
{
};

class TypeVisitor : public Visitor
{
  public:
    virtual void visit(const NeverType &type) = 0;
    virtual void visit(const SimpleType &type) = 0;
    virtual void visit(const FunctionType &type) = 0;
};

class AstVisitor : public Visitor
{
  public:
    virtual void visit(const VariableExpr &node) = 0;
    virtual void visit(const I32Expr &node) = 0;
    virtual void visit(const F64Expr &node) = 0;
    virtual void visit(const StringExpr &node) = 0;
    virtual void visit(const CharExpr &node) = 0;
    virtual void visit(const BoolExpr &node) = 0;
    virtual void visit(const BinaryExpr &node) = 0;
    virtual void visit(const UnaryExpr &node) = 0;
    virtual void visit(const CallExpr &node) = 0;
    virtual void visit(const LambdaCallExpr &node) = 0;
    virtual void visit(const Block &node) = 0;
    virtual void visit(const IfStmt &node) = 0;
    virtual void visit(const WhileStmt &node) = 0;
    virtual void visit(const MatchStmt &node) = 0;
    virtual void visit(const LiteralArm &node) = 0;
    virtual void visit(const GuardArm &node) = 0;
    virtual void visit(const PlaceholderArm &node) = 0;
    virtual void visit(const ReturnStmt &node) = 0;
    virtual void visit(const BreakStmt &node) = 0;
    virtual void visit(const ContinueStmt &node) = 0;
    virtual void visit(const FuncDefStmt &node) = 0;
    virtual void visit(const AssignStmt &node) = 0;
    virtual void visit(const VarDeclStmt &node) = 0;
    virtual void visit(const ExternStmt &node) = 0;
    virtual void visit(const Program &node) = 0;
    virtual void visit(const Parameter &node) = 0;
};
#endif