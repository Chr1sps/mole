#ifndef __VISITOR_HPP__
#define __VISITOR_HPP__
#include "ast.hpp"

class Visitor
{
  public:
    virtual void visit(const VariableExpr &node) = 0;
    virtual void visit(const I32Expr &node) = 0;
    virtual void visit(const F32Expr &node) = 0;
    virtual void visit(const F64Expr &node) = 0;
    virtual void visit(const BinaryExpr &node) = 0;
    virtual void visit(const UnaryExpr &node) = 0;
    virtual void visit(const CallExpr &node) = 0;
    virtual void visit(const Block &node) = 0;
    virtual void visit(const ReturnStmt &node) = 0;
    virtual void visit(const FuncDefStmt &node) = 0;
    virtual void visit(const AssignStmt &node) = 0;
    virtual void visit(const VarDeclStmt &node) = 0;
    virtual void visit(const ExternStmt &node) = 0;
    virtual void visit(const Program &node) = 0;
};
#endif