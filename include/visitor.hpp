#ifndef __VISITOR_HPP__
#define __VISITOR_HPP__

struct VariableExpr;
struct I32Expr;
struct F64Expr;
struct BinaryExpr;
struct UnaryExpr;
struct CallExpr;
struct LambdaCallExpr;
struct Block;
struct ReturnStmt;
struct FuncDefStmt;
struct AssignStmt;
struct VarDeclStmt;
struct ExternStmt;
struct Program;

struct NeverType;
struct SimpleType;
struct FunctionType;

struct BuiltInBinOp;
struct BuiltInUnaryOp;

class Visitor
{
  public:
    virtual void visit(const VariableExpr &node) = 0;
    virtual void visit(const I32Expr &node) = 0;
    virtual void visit(const F64Expr &node) = 0;
    virtual void visit(const BinaryExpr &node) = 0;
    virtual void visit(const UnaryExpr &node) = 0;
    virtual void visit(const CallExpr &node) = 0;
    virtual void visit(const LambdaCallExpr &node) = 0;
    virtual void visit(const Block &node) = 0;
    virtual void visit(const ReturnStmt &node) = 0;
    virtual void visit(const FuncDefStmt &node) = 0;
    virtual void visit(const AssignStmt &node) = 0;
    virtual void visit(const VarDeclStmt &node) = 0;
    virtual void visit(const ExternStmt &node) = 0;
    virtual void visit(const Program &node) = 0;

    virtual void visit(const NeverType &type) = 0;
    virtual void visit(const SimpleType &type) = 0;
    virtual void visit(const FunctionType &type) = 0;

    virtual void visit(const BuiltInBinOp &op) = 0;
    virtual void visit(const BuiltInUnaryOp &op) = 0;
};
#endif