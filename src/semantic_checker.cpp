#include "semantic_checker.hpp"
#include "ast.hpp"
#include "exceptions.hpp"

void SemanticChecker::enter_scope()
{
    this->functions.push_back({});
    this->variables.push_back({});
}

void SemanticChecker::leave_scope()
{
    this->functions.pop_back();
    this->variables.pop_back();
}

void SemanticChecker::register_globals(const Program &node)
{
    for (auto &var : node.globals)
        var->accept(*this);
    for (auto &ext : node.externs)
        ext->accept(*this);
    for (auto &func : node.functions)
        func->accept(*this);
}

void SemanticChecker::check_function_names(const VarDeclStmt &node)
{
    for (auto &scope_funcs : this->functions)
    {
        for (auto &func : scope_funcs)
        {
            if (node.name == func.name)
            {
                this->report_error(
                    L"defined variable has the same name as a function");
            }
        }
    }
}

void SemanticChecker::check_variable_names(const VarDeclStmt &node)
{
    for (auto &scope_vars : this->variables)
    {
        for (auto &var : scope_vars)
        {
            if (node.name == var.name)
            {
                this->report_error(
                    L"defined variable has the same name as another variable");
            }
        }
    }
}

void SemanticChecker::check_main(const VarDeclStmt &node)
{
    if (node.name == L"main")
        this->report_error(L"variable cannot be named 'main'");
}

void SemanticChecker::check_var_decl_type_match(const VarDeclStmt &node)
{
    if (node.type && node.initial_value)
    {
        (*(node.initial_value))->accept(*this);
        auto value_type = this->last_type;
        if (*(*(node.type)) != *(value_type))
            this->report_error(L"variable's declared type and assigned "
                               L"value's type don't match");
    }
}

void SemanticChecker::report_error(const std::wstring &msg)
{
    throw SemanticException(msg);
}

void SemanticChecker::visit(const VariableExpr &node)
{
}

void SemanticChecker::visit(const I32Expr &node)
{
    this->last_type = I32Expr::type;
}

void SemanticChecker::visit(const F64Expr &node)
{
    this->last_type = F64Expr::type;
}

void SemanticChecker::visit(const BinaryExpr &node)
{
    node.lhs->accept(*this);
    auto left_type = this->last_type;

    node.rhs->accept(*this);
    auto right_type = this->last_type;
    if (*left_type != *right_type)
    {
        this->report_error(L"binary operator type mismatch");
    }

    this->last_type = left_type;
}

void SemanticChecker::visit(const UnaryExpr &node)
{
    node.expr->accept(*this);
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
            this->report_error(L"wrong main function return type declaration");
        }
        if (*node.return_type == SimpleType(TypeEnum::U8))
            node.block->accept(*this);
    }
}

void SemanticChecker::visit(const AssignStmt &node)
{
}

void SemanticChecker::visit(const VarDeclStmt &node)
{
    this->check_variable_names(node);
    this->check_function_names(node);
    if (node.is_mut)
    {
        if (!(node.initial_value || node.type))
        {
            this->report_error(
                L"mutable must have either a type or a value assigned to it");
        }
    }
    else if (!node.initial_value)
    {
        this->report_error(L"constant must have a value assigned to it");
    }
    this->check_var_decl_type_match(node);
    this->check_main(node);
}

void SemanticChecker::visit(const ExternStmt &node)
{
}

void SemanticChecker::visit(const Program &node)
{
    this->enter_scope();
    for (auto &var : node.globals)
        var->accept(*this);
    for (auto &ext : node.externs)
        ext->accept(*this);
    for (auto &func : node.functions)
        func->accept(*this);
    this->leave_scope();
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
