#ifndef __VISITOR_HPP__
#define __VISITOR_HPP__
#include "ast.hpp"
#include "overloaded.hpp"

class TypeVisitor
{
  public:
    virtual void visit(const Type &type) = 0;
};

class ExprVisitor
{
  public:
    virtual void visit(const Expression &node) = 0;
};

class StmtVisitor
{
  public:
    virtual void visit(const Statement &node) = 0;
};

class MatchArmVisitor
{
  public:
    virtual void visit(const MatchArm &node) = 0;
};

class ParamVisitor
{
  public:
    virtual void visit(const Parameter &node) = 0;
};

class ProgramVisitor
{
  public:
    virtual void visit(const Program &node) = 0;
};

class AstVisitor : public ExprVisitor,
                   public StmtVisitor,
                   public MatchArmVisitor,
                   public TypeVisitor,
                   public ParamVisitor,
                   public ProgramVisitor
{
  public:
    using ExprVisitor::visit;
    using MatchArmVisitor::visit;
    using ParamVisitor::visit;
    using ProgramVisitor::visit;
    using StmtVisitor::visit;
    using TypeVisitor::visit;
};
#endif