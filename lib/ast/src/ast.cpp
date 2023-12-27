#include "ast.hpp"
const std::shared_ptr<SimpleType> U32Expr::type =
    std::make_shared<SimpleType>(TypeEnum::I32, RefSpecifier::NON_REF);
const std::shared_ptr<SimpleType> F64Expr::type =
    std::make_shared<SimpleType>(TypeEnum::F64, RefSpecifier::NON_REF);

VariableExpr::VariableExpr(const std::wstring &name,
                           const Position &position) noexcept
    : AstNode(position), name(name)
{
}

BinaryExpr::BinaryExpr(ExprNodePtr lhs, ExprNodePtr rhs, const BinOpEnum &op,
                       const Position &position) noexcept
    : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), op(op)
{
}

UnaryExpr::UnaryExpr(ExprNodePtr expr, const UnaryOpEnum &op,
                     const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)), op(op)
{
}

CallExpr::CallExpr(ExprNodePtr callable, std::vector<ExprNodePtr> args,
                   const Position &position) noexcept
    : AstNode(position), callable(std::move(callable)), args(std::move(args))
{
}

LambdaCallExpr::LambdaCallExpr(ExprNodePtr callable,
                               std::vector<ExprNodePtr> args,
                               const Position &position) noexcept
    : AstNode(position), callable(std::move(callable)), args(std::move(args))
{
}

IndexExpr::IndexExpr(ExprNodePtr expr, ExprNodePtr index_value,
                     const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)),
      index_value(std::move(index_value))
{
}

CastExpr::CastExpr(ExprNodePtr expr, TypePtr type,
                   const Position &position) noexcept
    : AstNode(position), expr(std::move(expr)), type(std::move(type))
{
}

U32Expr::U32Expr(const unsigned long long &value,
                 const Position &position) noexcept
    : AstNode(position), value(value)
{
}

Block::Block(std::vector<StmtPtr> statements,
             const Position &position) noexcept
    : AstNode(position), statements(std::move(statements))
{
}

ReturnStmt::ReturnStmt(const Position &position) noexcept
    : AstNode(position), expr()
{
}

ReturnStmt::ReturnStmt(ExprNodePtr expr, const Position &position) noexcept
    : AstNode(position), expr(std::move(expr))
{
}

ContinueStmt::ContinueStmt(const Position &position) noexcept
    : AstNode(position)
{
}

BreakStmt::BreakStmt(const Position &position) noexcept : AstNode(position)
{
}

VarDeclStmt::VarDeclStmt(const std::wstring &name, TypePtr type,
                         ExprNodePtr value, const bool &is_mut,
                         const Position &position) noexcept
    : AstNode(position), name(name), type(std::move(type)),
      initial_value(std::move(value)), is_mut(is_mut)
{
}

AssignStmt::AssignStmt(ExprNodePtr lhs, const AssignType &type,
                       ExprNodePtr rhs, const Position &position) noexcept
    : AstNode(position), lhs(std::move(lhs)), rhs(std::move(rhs)), type(type)
{
}

ExprStmt::ExprStmt(ExprNodePtr expr, const Position &position) noexcept
    : AstNode(position), expr(std::move(expr))
{
}

WhileStmt::WhileStmt(ExprNodePtr condition_expr, StmtPtr statement,
                     const Position &position) noexcept
    : AstNode(position), condition_expr(std::move(condition_expr)),
      statement(std::move(statement))
{
}

IfStmt::IfStmt(ExprNodePtr condition_expr, StmtPtr then_block,
               StmtPtr else_block, const Position &position) noexcept
    : AstNode(position), condition_expr(std::move(condition_expr)),
      then_block(std::move(then_block)), else_block(std::move(else_block))
{
}

MatchStmt::MatchStmt(ExprNodePtr matched_expr,
                     std::vector<MatchArmPtr> match_arms,
                     const Position &position) noexcept
    : AstNode(position), matched_expr(std::move(matched_expr)),
      match_arms(std::move(match_arms))
{
}

FuncDefStmt::FuncDefStmt(const std::wstring &name,
                         std::vector<ParamPtr> params, TypePtr return_type,
                         BlockPtr block, const bool &is_const,
                         const Position &position) noexcept
    : AstNode(position), name(name), params(std::move(params)),
      return_type(std::move(return_type)), block(std::move(block)),
      is_const(is_const)
{
}

ExternStmt::ExternStmt(const std::wstring &name, std::vector<ParamPtr> params,
                       TypePtr return_type, const Position &position) noexcept
    : AstNode(position), name(name), params(std::move(params)),
      return_type(std::move(return_type))
{
}

Program::Program(std::vector<std::unique_ptr<VarDeclStmt>> globals,
                 std::vector<std::unique_ptr<FuncDefStmt>> functions,
                 std::vector<std::unique_ptr<ExternStmt>> externs) noexcept
    : AstNode(Position(1, 1)), globals(std::move(globals)),
      functions(std::move(functions)), externs(std::move(externs))
{
}

MatchArmBase::MatchArmBase(StmtPtr block, const Position &position) noexcept
    : AstNode(position), block(std::move(block))
{
}

GuardArm::GuardArm(ExprNodePtr condition_expr, StmtPtr block,
                   const Position &position) noexcept
    : MatchArmBase(std::move(block), position),
      condition_expr(std::move(condition_expr))
{
}

LiteralArm::LiteralArm(std::vector<ExprNodePtr> literals, StmtPtr block,
                       const Position &position) noexcept
    : MatchArmBase(std::move(block), position), literals(std::move(literals))
{
}

ElseArm::ElseArm(StmtPtr block, const Position &position) noexcept
    : MatchArmBase(std::move(block), position)
{
}

Parameter::Parameter(const std::wstring &name, TypePtr type,
                     const Position &position) noexcept
    : AstNode(position), name(name), type(std::move(type))
{
}