#include "semantic_checker.hpp"
#include "ast.hpp"
#include "exceptions.hpp"
#include "string_builder.hpp"

void SemanticChecker::enter_scope()
{
    ++this->scope_level;
    this->functions.push_back({});
    this->variables.push_back({});
}

void SemanticChecker::leave_scope()
{
    --this->scope_level;
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
            if (node.name == func->name)
            {
                this->report_error(
                    L"defined variable has the same name as a function");
            }
        }
    }
}

void SemanticChecker::check_function_params(const FuncDefStmt &node)
{
    for (auto &param : node.params)
    {
        if (auto var = this->find_variable(param->name))
            this->report_error(L"param name cannot shadow a variable that is "
                               L"already in scope");
        if (auto func = this->find_function(param->name))
            this->report_error(L"param name cannot shadow a function that is "
                               L"already in scope");
    }
}

void SemanticChecker::check_function_block(const FuncDefStmt &node)
{
    this->const_scopes.push_back(this->scope_level);
    node.block->accept(*this);
    this->const_scopes.pop_back();
}

void SemanticChecker::check_variable_names(const VarDeclStmt &node)
{
    for (auto &scope_vars : this->variables)
    {
        for (auto &var : scope_vars)
        {
            if (node.name == var->name)
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

void SemanticChecker::check_var_value_and_type(const VarDeclStmt &node)
{
    if (node.initial_value)
    {
        (*(node.initial_value))->accept(*this);
        if (node.type)
        {
            auto value_type = this->last_type;
            if (*(*(node.type)) != *(value_type))
                this->report_error(L"variable's declared type and assigned "
                                   L"value's type don't match");
        }
    }
}

std::shared_ptr<SemanticChecker::Variable> SemanticChecker::find_variable(
    const std::wstring &name)
{
    for (auto &scope : this->variables)
    {
        auto found = std::find_if(scope.begin(), scope.end(),
                                  [name](std::shared_ptr<Variable> var) {
                                      return var->name == name;
                                  });
        if (found != scope.end())
            return *found;
    }
    return nullptr;
}

std::shared_ptr<SemanticChecker::Function> SemanticChecker::find_function(
    const std::wstring &name)
{
    for (auto &scope : this->functions)
    {
        auto found = std::find_if(scope.begin(), scope.end(),
                                  [name](std::shared_ptr<Function> foo) {
                                      return foo->name == name;
                                  });
        if (found != scope.end())
            return *found;
    }
    return nullptr;
}

std::shared_ptr<SemanticChecker::Variable> SemanticChecker::
    find_outside_variable(const std::wstring &name)
{
    if (!this->const_scopes.empty())
    {

        for (size_t i = 0;
             i < this->variables.size() && i < this->const_scopes.back(); ++i)
        {
            auto scope = this->variables[i];
            auto found = std::find_if(scope.begin(), scope.end(),
                                      [name](std::shared_ptr<Variable> var) {
                                          return var->name == name;
                                      });
            if (found != scope.end())
                return *found;
        }
        //     for (auto scope = this->variables.begin();
        //          scope < (this->variables.end() - 1) &&
        //          scope < (this->variables.begin() +
        //          this->const_scopes.back());
        //          ++scope)
        //     {
        //         auto found = std::find_if(scope->begin(), scope->end(),
        //                                   [name](std::shared_ptr<Variable>
        //                                   var) {
        //                                       return var->name == name;
        //                                   });
        //         if (found != scope->end())
        //             return *found;
        //     }
    }
    return nullptr;
}

std::shared_ptr<SemanticChecker::Function> SemanticChecker::
    find_outside_function(const std::wstring &name)
{
    for (auto scope = this->functions.begin();
         scope < (this->functions.end() - 1); ++scope)
    {
        auto found = std::find_if(scope->begin(), scope->end(),
                                  [name](std::shared_ptr<Function> foo) {
                                      return foo->name == name;
                                  });
        if (found != scope->end())
            return *found;
    }
    return nullptr;
}

void SemanticChecker::register_local_variable(const VarDeclStmt &node)
{
    auto new_var = Variable(node.name, this->last_type, node.is_mut,
                            node.initial_value.has_value());
    this->variables.back().insert(std::make_shared<Variable>(new_var));
}

void SemanticChecker::register_local_function(const FuncDefStmt &node)
{
    std::vector<TypePtr> param_types;

    for (auto &param : node.params)
    {
        param_types.push_back(param->type);
    }

    auto fn_type = std::make_shared<FunctionType>(
        param_types, node.return_type, node.is_const);

    auto new_func = Function(node.name, fn_type);

    this->functions.back().insert(std::make_shared<Function>(new_func));
}

void SemanticChecker::visit(const VariableExpr &node)
{
    auto variable = this->find_variable(node.name);
    // auto func = this->find_function(node.name);
    if (!variable)
        this->report_error(L"variable `", node.name, L"` not found in scope");
    if (!variable->initialized)
        this->report_error(L"variable `", node.name, L"` is not initialized");
    if (variable = this->find_outside_variable(node.name))
    {
        this->report_error(L"variable `", node.name,
                           L"` is accessed in a const function");
    }
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
    auto func = this->find_function(node.func_name);
    if (!func)
        this->report_error(L"function `", node.func_name,
                           L"` not found in scope");
    if (node.args.size() != func->type->arg_types.size())
        this->report_error(L"wrong amount of arguments in a function call");
    for (size_t i = 0; i < func->type->arg_types.size(); ++i)
    {
        auto expected_type = func->type->arg_types[i];
        (node.args[i])->accept(*this);
        auto actual_type = this->last_type;
        if (expected_type != actual_type)
            this->report_error(L"function call argument type is mismatched");
    }
}

void SemanticChecker::visit(const LambdaCallExpr &node)
{
}

void SemanticChecker::visit(const Block &node)
{
    this->enter_scope();
    for (auto &stmt : node.statements)
        stmt->accept(*this);
    this->leave_scope();
}

void SemanticChecker::visit(const ReturnStmt &node)
{
    if (!this->return_type)
    {
        if (node.expr)
        {
            (*(node.expr))->accept(*this);
            this->return_type = this->last_type;
        }
        else
            this->return_type = std::make_shared<NeverType>();
    }
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
        if (!node.params.empty())
        {
            this->report_error(L"main cannot have any parameters");
        }
    }

    this->check_function_params(node);

    this->check_function_block(node);

    if (!this->return_type && (*(node.return_type) != NeverType()))
        this->report_error(L"function doesn't return when it should");
    if (this->return_type && (*(this->return_type) != *(node.return_type)))
        this->report_error(L"function returns the wrong type");

    this->register_local_function(node);
}

void SemanticChecker::visit(const AssignStmt &node)
{
    if (auto var = this->find_variable(node.name))
    {
        if (!(var->mut))
        {
            this->report_error(L"constant value cannot be reassigned");
        }

        if (this->find_outside_variable(node.name))
        {
            this->report_error(L"variable `", node.name,
                               "` is reassigned in a constant function");
        }

        auto var_type = var->type;
        node.value->accept(*this);
        auto assigned_type = this->last_type;
        if (var_type != assigned_type)
            this->report_error(
                L"mismatched types in an assignment expression");
    }
    else
    {
        this->report_error(L"reassigned variable is not in scope");
    }
}

void SemanticChecker::visit(const VarDeclStmt &node)
{
    this->check_variable_names(node);
    this->check_function_names(node);
    if (node.is_mut)
    {
        if (!(node.initial_value || node.type))
        {
            this->report_error(L"mutable must have either a type or a "
                               L"value assigned to it");
        }
    }
    else if (!node.initial_value)
    {
        this->report_error(L"constant must have a value assigned to it");
    }
    this->check_main(node);
    this->check_var_value_and_type(node);
    this->register_local_variable(node);
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
