#include "semantic_checker.hpp"
#include "ast.hpp"

void SemanticChecker::visit(const VariableExpr &node)
{
}

void SemanticChecker::visit(const I32Expr &node)
{
}

void SemanticChecker::visit(const F32Expr &node)
{
}

void SemanticChecker::visit(const F64Expr &node)
{
}

void SemanticChecker::visit(const BinaryExpr &node)
{
}

void SemanticChecker::visit(const UnaryExpr &node)
{
}

void SemanticChecker::visit(const CallExpr &node)
{
}

void SemanticChecker::visit(const LambdaCallExpr &node)
{
}

void SemanticChecker::visit(const Block &node)
{
    for (auto &stmt : node.statements)
        stmt->accept(*this);
}

void SemanticChecker::visit(const ReturnStmt &node)
{
}

void SemanticChecker::visit(const FuncDefStmt &node)
{
}

void SemanticChecker::visit(const AssignStmt &node)
{
}

void SemanticChecker::visit(const VarDeclStmt &node)
{
}

void SemanticChecker::visit(const ExternStmt &node)
{
}

void SemanticChecker::visit(const Program &node)
{
}

void SemanticChecker::visit(const NeverType &type)
{
}

void SemanticChecker::visit(const SimpleType &type)
{
}

void SemanticChecker::visit(const FunctionType &type)
{
}

void SemanticChecker::visit(const BuiltInBinOp &op)
{
}

void SemanticChecker::visit(const BuiltInUnaryOp &op)
{
}
