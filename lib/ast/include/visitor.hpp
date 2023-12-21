#ifndef __VISITOR_HPP__
#define __VISITOR_HPP__
// #include <variant>
// struct VariableExpr;
// struct I32Expr;
// struct F64Expr;
// struct StringExpr;
// struct CharExpr;
// struct BoolExpr;
// struct BinaryExpr;
// struct UnaryExpr;
// struct CallExpr;
// struct LambdaCallExpr;
// struct IndexExpr;
// struct CastExpr;
// using ExprNode = std::variant<VariableExpr, BinaryExpr, UnaryExpr, CallExpr,
//                               LambdaCallExpr, IndexExpr, CastExpr, I32Expr,
//                               F64Expr, BoolExpr, StringExpr, CharExpr>;

// struct Block;
// struct IfStmt;
// struct WhileStmt;
// struct MatchStmt;
// struct BreakStmt;
// struct ContinueStmt;
// struct ReturnStmt;
// struct FuncDefStmt;
// struct AssignStmt;
// struct ExprStmt;
// struct VarDeclStmt;
// struct ExternStmt;
// using Statement = std::variant<Block, ReturnStmt, ContinueStmt, BreakStmt,
//                                VarDeclStmt, AssignStmt, ExprStmt, WhileStmt,
//                                IfStmt, MatchStmt, FuncDefStmt, ExternStmt>;

struct LiteralArm;
struct GuardArm;
struct ElseArm;
struct Program;
struct Parameter;

struct SimpleType;
struct FunctionType;

class Visitor
{
};

class TypeVisitor : public Visitor
{
  public:
    virtual void visit(const SimpleType &type) = 0;
    virtual void visit(const FunctionType &type) = 0;
};

// class ExprVisitor : Visitor
// {
//   public:
//     virtual void visit(const ExprNode &node) = 0;
// };

// class StmtVisitor : Visitor
// {
//   public:
//     virtual void visit(const Statement &node) = 0;
//     // virtual void visit(const Block &node) = 0;
//     // virtual void visit(const IfStmt &node) = 0;
//     // virtual void visit(const WhileStmt &node) = 0;
//     // virtual void visit(const MatchStmt &node) = 0;
//     // virtual void visit(const ReturnStmt &node) = 0;
//     // virtual void visit(const BreakStmt &node) = 0;
//     // virtual void visit(const ContinueStmt &node) = 0;
//     // virtual void visit(const FuncDefStmt &node) = 0;
//     // virtual void visit(const AssignStmt &node) = 0;
//     // virtual void visit(const ExprStmt &node) = 0;
//     // virtual void visit(const VarDeclStmt &node) = 0;
//     // virtual void visit(const ExternStmt &node) = 0;
// };

class MatchArmVisitor : Visitor
{
  public:
    virtual void visit(const LiteralArm &node) = 0;
    virtual void visit(const GuardArm &node) = 0;
    virtual void visit(const ElseArm &node) = 0;
};

class AstVisitor :
    //  public ExprVisitor,
    //  public StmtVisitor,
    public MatchArmVisitor,
    public TypeVisitor
{
  public:
    virtual void visit(const Program &node) = 0;
    virtual void visit(const Parameter &node) = 0;
    // using ExprVisitor::visit;
    using MatchArmVisitor::visit;
    // using StmtVisitor::visit;
    using TypeVisitor::visit;
};
#endif