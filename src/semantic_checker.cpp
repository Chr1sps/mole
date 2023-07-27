#include "semantic_checker.hpp"
#include "ast.hpp"
#include "exceptions.hpp"

void SemanticChecker::visit(const VariableExpr &node)
{
}

void SemanticChecker::visit(const I32Expr &node)
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
    if (node.name == L"main")
    {
        if (!(*node.return_type == NeverType() ||
              *node.return_type == SimpleType(TypeEnum::U8)))
        {
            throw SemanticException(
                L"wrong main function return type declaration");
        }
        if (*node.return_type == SimpleType(TypeEnum::U8) &&
            node.block->statements.empty())
        {
            throw SemanticException(
                L"main doesn't return a u8 value, when it should");
        }
    }
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
    for (auto &func : node.functions)
        func->accept(*this);
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
