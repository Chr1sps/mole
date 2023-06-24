#ifndef __VISITOR_HPP__
#define __VISITOR_HPP__
#include "ast.hpp"
class Visitor
{
public:
    virtual void visit(const VariableExprNode &node) = 0;
    virtual void visit(const I32ConstNode &node) = 0;
    virtual void visit(const F32ConstNode &node) = 0;
    virtual void visit(const F64ConstNode &node) = 0;
};
#endif