#ifndef __VISITOR_HPP__
#define __VISITOR_HPP__
#include "ast.hpp"
#include "overloaded.hpp"

class Visitor
{
};

class TypeVisitor : public Visitor
{
  public:
    virtual void visit(const Type &type) = 0;
};

class ExprVisitor : Visitor
{
  public:
    virtual void visit(const Expression &node) = 0;
};

class StmtVisitor : Visitor
{
  public:
    virtual void visit(const Statement &node) = 0;
};

class MatchArmVisitor : Visitor
{
  public:
    virtual void visit(const MatchArm &node) = 0;
};

class AstVisitor : public ExprVisitor,
                   public StmtVisitor,
                   public MatchArmVisitor,
                   public TypeVisitor
{
  public:
    virtual void visit(const Program &node) = 0;
    virtual void visit(const Parameter &node) = 0;
    using ExprVisitor::visit;
    using MatchArmVisitor::visit;
    using StmtVisitor::visit;
    using TypeVisitor::visit;
};
#endif