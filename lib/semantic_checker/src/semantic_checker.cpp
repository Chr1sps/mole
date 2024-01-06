#include "semantic_checker.hpp"
#include "ast.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <optional>

const std::unordered_multimap<TypeEnum, TypeEnum>
    SemanticChecker::Visitor::cast_map = {
        {TypeEnum::BOOL, TypeEnum::BOOL}, {TypeEnum::BOOL, TypeEnum::U32},
        {TypeEnum::BOOL, TypeEnum::I32},  {TypeEnum::BOOL, TypeEnum::F64},
        {TypeEnum::U32, TypeEnum::U32},   {TypeEnum::U32, TypeEnum::I32},
        {TypeEnum::U32, TypeEnum::F64},   {TypeEnum::U32, TypeEnum::CHAR},
        {TypeEnum::I32, TypeEnum::U32},   {TypeEnum::I32, TypeEnum::I32},
        {TypeEnum::I32, TypeEnum::F64},   {TypeEnum::I32, TypeEnum::CHAR},
        {TypeEnum::F64, TypeEnum::U32},   {TypeEnum::F64, TypeEnum::I32},
        {TypeEnum::F64, TypeEnum::F64},   {TypeEnum::CHAR, TypeEnum::U32},
        {TypeEnum::CHAR, TypeEnum::I32},  {TypeEnum::CHAR, TypeEnum::CHAR},
};

SemanticChecker::Visitor::Visitor() noexcept
    : last_type(nullptr), return_type(nullptr), is_assignable(false),
      value(true)
{
}

void SemanticChecker::Visitor::enter_scope()
{
    this->function_map.push_back({});
    this->variable_map.push_back({});
}

void SemanticChecker::Visitor::leave_scope()
{
    this->function_map.pop_back();
    this->variable_map.pop_back();
}

// void SemanticChecker::Visitor::register_globals(const Program &node)
// {
//     for (auto &var : node.globals)
//         var->accept(*this);
//     for (auto &ext : node.externs)
//         ext->accept(*this);
//     for (auto &func : node.functions)
//         func->accept(*this);
// }

// void SemanticChecker::Visitor::check_function_names(const VarDeclStmt &node)
// {
//     for (auto &scope_funcs : this->functions)
//     {
//         for (auto &func : scope_funcs)
//         {
//             if (node.name == func->name)
//             {
//                 this->report_error(
//                     L"defined variable has the same name as a function");
//             }
//         }
//     }
// }

// void SemanticChecker::Visitor::check_function_params(const FuncDefStmt
// &node)
// {
//     for (auto &param : node.params)
//     {
//         if (auto var = this->find_variable(param->name))
//             this->report_error(L"param name cannot shadow a variable that is
//             "
//                                L"already in scope");
//         if (auto func = this->find_function(param->name))
//             this->report_error(L"param name cannot shadow a function that is
//             "
//                                L"already in scope");
//     }
// }

// void SemanticChecker::Visitor::check_function_params(const ExternStmt &node)
// {
//     for (auto &param : node.params)
//     {
//         if (auto var = this->find_variable(param->name))
//             this->report_error(L"param name cannot shadow a variable that is
//             "
//                                L"already in scope");
//         if (auto func = this->find_function(param->name))
//             this->report_error(L"param name cannot shadow a function that is
//             "
//                                L"already in scope");
//     }
// }

// void SemanticChecker::Visitor::check_function_block(const FuncDefStmt &node)
// {
//     if (node.is_const)
//         this->const_scopes.push_back(this->scope_level);
//     node.block->accept(*this);
//     if (node.is_const)
//         this->const_scopes.pop_back();
// }

// void SemanticChecker::Visitor::check_function_return(const FuncDefStmt
// &node)
// {
//     auto return_type = this->return_stack.back();
//     if (!return_type && (*(node.return_type) != NeverType()))
//         this->report_error(L"function doesn't return when it should");
//     if (return_type && (*(return_type) != *(node.return_type)))
//         this->report_error(L"function returns the wrong type");
// }

// void SemanticChecker::Visitor::check_variable_names(const VarDeclStmt &node)
// {
//     for (auto &scope_vars : this->variables)
//     {
//         for (auto &var : scope_vars)
//         {
//             if (node.name == var->name)
//             {
//                 this->report_error(
//                     L"defined variable has the same name as another
//                     variable");
//             }
//         }
//     }
// }

void SemanticChecker::Visitor::check_main_function(const FuncDefStmt &node)
{
    if (!(!node.return_type ||
          *node.return_type ==
              SimpleType(TypeEnum::U32, RefSpecifier::NON_REF)))
    {
        this->report_error(L"wrong main function return type declaration");
    }
    // if (*node.return_type == SimpleType(TypeEnum::U32,
    // RefSpecifier::NON_REF))
    //     node.block->accept(*this);
    if (!node.params.empty())
    {
        this->report_error(L"main cannot have any parameters");
    }
}

// void SemanticChecker::Visitor::check_var_name(const VarDeclStmt &node)
// {
//     if (node.name == L"main")
//         this->report_error(L"variable cannot be named 'main'");
// }

bool SemanticChecker::Visitor::check_var_value_and_type(
    const VarDeclStmt &node)
{
    if (node.initial_value)
    {
        this->visit(*node.initial_value);
        if (!this->last_type)
        {
            return false;
        }
        if (node.type)
        {
            auto &value_type = this->last_type;
            if (!((!value_type && !node.type) ||
                  (value_type && node.type && *value_type == *node.type)))
            {
                this->report_error(L"variable's declared type and assigned "
                                   L"value's type don't match");
                return false;
            }
        }
        if (!this->is_initialized)
        {
            this->report_error(
                L"initial value expression in a variable declaration contains"
                L"an uninitialized variable");
            return false;
        }
    }
    return true;
}

auto SemanticChecker::Visitor::find_variable(const std::wstring &name)
    -> std::optional<VarData>
{
    for (const auto &scope : this->variable_map)
    {
        auto found = scope.find(name);
        if (found != scope.end())
            return found->second;
    }
    return std::nullopt;
}

// std::shared_ptr<SemanticChecker::Function> SemanticChecker::Visitor::
//     find_function(const std::wstring &name)
// {
//     for (auto &scope : this->functions)
//     {
//         auto found = std::find_if(scope.begin(), scope.end(),
//                                   [name](std::shared_ptr<Function> foo) {
//                                       return foo->name == name;
//                                   });
//         if (found != scope.end())
//             return *found;
//     }
//     return nullptr;
// }

// std::shared_ptr<SemanticChecker::Variable> SemanticChecker::Visitor::
//     find_outside_variable(const std::wstring &name)
// {
//     if (!this->const_scopes.empty())
//     {
//         for (size_t i = 0;
//              i < this->variables.size() && i < this->const_scopes.back();
//              ++i)
//         {
//             auto scope = this->variables[i];
//             auto found = std::find_if(scope.begin(), scope.end(),
//                                       [&name](std::shared_ptr<Variable> var)
//                                       {
//                                           return var->name == name;
//                                       });
//             if (found != scope.end())
//                 return *found;
//         }
//     }
//     return nullptr;
// }

// std::shared_ptr<SemanticChecker::Function> SemanticChecker::Visitor::
//     find_outside_function(const std::wstring &name)
// {
//     for (auto scope = this->functions.begin();
//          scope < (this->functions.end() - 2); ++scope)
//     {
//         auto found = std::find_if(scope->begin(), scope->end(),
//                                   [&name](std::shared_ptr<Function> foo) {
//                                       return foo->name == name;
//                                   });
//         if (found != scope->end())
//             return *found;
//     }
//     return nullptr;
// }

void SemanticChecker::Visitor::register_local_variable(const VarDeclStmt &node)
{
    auto type = (node.type) ? (*node.type) : (*this->last_type);
    auto new_data = VarData(type, node.is_mut, node.initial_value != nullptr);
    this->variable_map.back().emplace(std::make_pair(node.name, new_data));
}

// void SemanticChecker::Visitor::register_local_function(const FuncDefStmt
// &node)
// {
//     std::vector<TypePtr> param_types;

//     for (auto &param : node.params)
//     {
//         param_types.push_back(param->type);
//     }

//     auto fn_type = std::make_shared<FunctionType>(
//         param_types, node.return_type, node.is_const);

//     auto new_func = Function(node.name, fn_type);

//     this->functions.back().insert(std::make_shared<Function>(new_func));
// }

// void SemanticChecker::Visitor::register_local_function(const ExternStmt
// &node)
// {
//     std::vector<TypePtr> param_types;

//     for (auto &param : node.params)
//     {
//         param_types.push_back(param->type);
//     }

//     auto fn_type =
//         std::make_shared<FunctionType>(param_types, node.return_type,
//         false);

//     auto new_func = Function(node.name, fn_type);

//     this->functions.back().insert(std::make_shared<Function>(new_func));
// }

void SemanticChecker::Visitor::register_function_params(
    const FuncDefStmt &node)
{
    for (auto &param : node.params)
    {
        this->visit(*param);
        auto type = *this->last_type;
        auto new_data = VarData(type, false, true);
        this->variable_map.back().emplace(
            std::make_pair(param->name, new_data));
    }
}

// void SemanticChecker::Visitor::unregister_function_params(
//     const FuncDefStmt &node)
// {
//     for (auto &param : node.params)
//     {
//         std::erase_if(this->variables.back(),
//                       [&param](const std::shared_ptr<Variable> &var) {
//                           return param->name == var->name;
//                       });
//     }
// }

// void SemanticChecker::Visitor::visit(const VariableExpr &node)
// {
//     auto variable = this->find_variable(node.name);
//     auto func = this->find_function(node.name);
//     if (variable)
//     {
//         if (!variable->initialized)
//             this->report_error(L"variable `", node.name,
//                                L"` is not initialized");
//         if (this->find_outside_variable(node.name))
//         {
//             this->report_error(L"variable `", node.name,
//                                L"` is accessed in a const function");
//         }
//         this->last_type = variable->type;
//     }
//     else if (func)
//     {
//         if (this->find_outside_function(node.name))
//         {
//             this->report_error(L"outside function `", node.name,
//                                L"` is accessed in a const function");
//         }
//         this->last_type = func->type;
//     }
//     else
//         this->report_error(L"variable `", node.name, L"` not found in
//         scope");
// }

// void SemanticChecker::Visitor::visit(const U32Expr &node)
// {
//     this->last_type = U32Expr::type;
// }

// void SemanticChecker::Visitor::visit(const F64Expr &node)
// {
//     this->last_type = F64Expr::type;
// }

// void SemanticChecker::Visitor::visit(const BinaryExpr &node)
// {
//     node.lhs->accept(*this);
//     auto left_type = this->last_type;

//     node.rhs->accept(*this);
//     auto right_type = this->last_type;
//     if (*left_type != *right_type)
//     {
//         this->report_error(L"binary operator type mismatch");
//     }

//     this->last_type = left_type;
// }
RefSpecifier get_ref_specifier(const UnaryOpEnum &op)
{
    switch (op)
    {
    case UnaryOpEnum::MUT_REF:
        return RefSpecifier::MUT_REF;
        break;

    default:
        return RefSpecifier::REF;
        break;
    }
}

void SemanticChecker::Visitor::visit(const UnaryExpr &node)
{
    this->visit(*node.expr);
    switch (node.op)
    {
    case UnaryOpEnum::INC:
    case UnaryOpEnum::DEC:
    case UnaryOpEnum::MINUS:
        break;

    case UnaryOpEnum::BIT_NEG:
        break;

    case UnaryOpEnum::NEG:
        break;

    case UnaryOpEnum::MUT_REF:
    case UnaryOpEnum::REF:
        std::visit(
            overloaded{
                [this, &node](const SimpleType &type) {
                    switch (type.ref_spec)
                    {
                    case RefSpecifier::NON_REF:
                        this->last_type = std::make_unique<Type>(
                            SimpleType(type.type, get_ref_specifier(node.op)));
                        break;

                    default:
                        this->report_error(L"references cannot be referenced");
                        break;
                    }
                },
                [this](const FunctionType &) {
                    this->report_error(
                        L"function references cannot be referenced");
                }},
            *this->last_type);
        break;
    case UnaryOpEnum::DEREF:
        break;
    }
}

// void SemanticChecker::Visitor::visit(const CallExpr &node)
// {
//     node.callable->accept(*this);
//     auto func = std::dynamic_pointer_cast<FunctionType>(this->last_type);
//     if (!func)
//     {
//         this->report_error(L"given function is not a callable");
//     }

//     if (node.args.size() != func->arg_types.size())
//         this->report_error(L"wrong amount of arguments in a function call");
//     for (size_t i = 0; i < func->arg_types.size(); ++i)
//     {
//         auto expected_type = func->arg_types[i];
//         (node.args[i])->accept(*this);
//         auto actual_type = this->last_type;
//         if (*expected_type != *actual_type)
//             this->report_error(L"function call argument type is
//             mismatched");
//     }
//     this->last_type = func->return_type;
// }

// void SemanticChecker::Visitor::visit(const LambdaCallExpr &node)
// {
//     node.callable->accept(*this);
//     auto func = std::dynamic_pointer_cast<FunctionType>(this->last_type);
//     if (!func)
//     {
//         this->report_error(L"given function is not a callable");
//     }

//     if (node.args.size() != func->arg_types.size())
//     {
//         if (!((node.args.size() < func->arg_types.size()) &&
//         node.is_ellipsis))
//             this->report_error(
//                 L"wrong amount of arguments in an in-place lambda");
//     }

//     auto new_args = std::vector<TypePtr>();
//     for (size_t i = 0; i < func->arg_types.size(); ++i)
//     {
//         auto expected_type = func->arg_types[i];
//         if (i < node.args.size())
//         {
//             auto &param_type = node.args[i];
//             if (param_type)
//             {
//                 (*param_type)->accept(*this);
//                 auto actual_type = this->last_type;
//                 if (*expected_type != *actual_type)
//                     this->report_error(
//                         L"in-place lambda argument type is mismatched");
//             }
//             else
//             {
//                 new_args.push_back(expected_type);
//             }
//         }
//         else
//         {
//             new_args.push_back(expected_type);
//         }
//     }
//     auto result_type = std::make_shared<FunctionType>(
//         new_args, func->return_type, func->is_const);
//     this->last_type = result_type;
// }

void SemanticChecker::Visitor::visit(const CastExpr &node)
{
    this->visit(*node.expr);
    auto to_type = std::get<SimpleType>((*node.type));
    std::visit(
        overloaded{
            [this, &to_type](const SimpleType &from_type) {
                if (from_type.ref_spec != RefSpecifier::NON_REF)
                {
                    this->report_error(L"cannot cast from a reference value");
                }
                if (to_type.ref_spec != RefSpecifier::NON_REF)
                {
                    this->report_error(L"cannot cast to a reference type");
                    this->last_type = nullptr;
                    return;
                }
                auto iter = std::find_if(
                    this->cast_map.cbegin(), this->cast_map.cend(),
                    [&from_type, &to_type](const auto &entry) {
                        return entry.first == from_type.type &&
                               entry.second == to_type.type;
                    });
                if (iter == this->cast_map.cend())
                {
                    this->report_error(
                        L"cast between two types not supported");
                    this->last_type = nullptr;
                }
                else
                {
                    this->last_type = std::make_unique<Type>(to_type);
                }
            },
            [this](const FunctionType &) {
                this->report_error(L"function references cannot be cast");
                this->last_type = nullptr;
            }},
        *this->last_type);
}

void SemanticChecker::Visitor::visit(const Block &node)
{
    this->enter_scope();
    for (auto &stmt : node.statements)
        this->visit(*stmt);
    this->leave_scope();
}

// void SemanticChecker::Visitor::visit(const ReturnStmt &node)
// {
//     if (!(this->return_stack.back()))
//     {
//         if (node.expr)
//         {
//             (*(node.expr))->accept(*this);
//             this->return_stack.back() = this->last_type;
//         }
//         else
//             this->return_stack.back() = std::make_shared<NeverType>();
//     }
// }

void SemanticChecker::Visitor::visit(const FuncDefStmt &node)
{
    if (node.name == L"main")
    {
        this->check_main_function(node);
    }

    // this->check_function_params(node);

    // this->return_stack.push_back(nullptr);
    // this->check_function_block(node);
    // this->visit(*node.block);
    this->enter_scope();
    // this->register_function_params(node);
    for (const auto &stmt : node.block->statements)
    {
        this->visit(*stmt);
    }
    this->leave_scope();
    // this->unregister_function_params(node);

    // this->check_function_return(node);
    // this->return_stack.pop_back();

    // this->register_local_function(node);
}

void SemanticChecker::Visitor::visit(const AssignStmt &node)
{
    this->visit(*node.lhs);
    if (!this->is_assignable)
    {
        this->report_error(
            L"left side of the assignment statement is non-assignable");
        return;
    }
    auto lhs_type = *this->last_type;
    this->visit(*node.rhs);
    auto rhs_type = *this->last_type;
    switch (node.type)
    {
    case AssignType::PLUS:
    case AssignType::MINUS:
    case AssignType::MUL:
    case AssignType::DIV:

    case AssignType::BIT_AND:
    case AssignType::BIT_OR:
    case AssignType::BIT_XOR:

    case AssignType::NORMAL:
        if (lhs_type != rhs_type)
        {
            this->report_error(L"assigned value type does not match the left "
                               L"side of the assignment statement");
        }
        if (!this->is_initialized)
        {
            this->report_error(L"expression assigned to a variable contains "
                               L"an uninitialized variable");
        }
        break;
    case AssignType::SHL:
    case AssignType::SHR:
        break;
    case AssignType::EXP:
        break;
    case AssignType::MOD:
        break;
    }

    // if (auto var = this->find_variable(node.name))
    // {
    //     if (!(var->mut))
    //     {
    //         this->report_error(L"constant value cannot be reassigned");
    //     }

    //     if (this->find_outside_variable(node.name))
    //     {
    //         this->report_error(L"variable `", node.name,
    //                            "` is reassigned in a constant function");
    //     }

    //     auto var_type = var->type;
    //     node.value->accept(*this);
    //     auto assigned_type = this->last_type;
    //     if (var_type != assigned_type)
    //         this->report_error(
    //             L"mismatched types in an assignment expression");
    // }
    // else
    // {
    //     this->report_error(L"reassigned variable is not in scope");
    // }
}

void SemanticChecker::Visitor::visit(const ExprStmt &node)
{
    this->visit(*node.expr);
}

void SemanticChecker::Visitor::visit(const VarDeclStmt &node)
{
    // this->check_variable_names(node);
    // this->check_function_names(node);
    auto registerable = true;
    if (node.is_mut)
    {
        if (!(node.initial_value || node.type))
        {
            this->report_error(L"mutable must have either a type or a "
                               L"value assigned to it");
            registerable = false;
        }
    }
    else if (!node.initial_value)
    {
        this->report_error(L"constant must have a value assigned to it");
        if (!node.type)
            registerable = false;
    }
    registerable &= this->check_var_value_and_type(node);
    // this->check_var_name(node);
    if (registerable)
        this->register_local_variable(node);
}

// void SemanticChecker::Visitor::visit(const ExternStmt &node)
// {
//     if (node.name == L"main")
//     {
//         this->report_error(L"`main` cannot be externed");
//     }

//     this->check_function_params(node);

//     this->register_local_function(node);
// }

void SemanticChecker::Visitor::visit(const Type &node)
{
}

void SemanticChecker::Visitor::visit(const Expression &node)
{
    std::visit(
        overloaded{
            [this](const U32Expr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::U32, RefSpecifier::NON_REF));
                this->is_assignable = false;
                this->is_initialized = true;
            },
            [this](const F64Expr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::F64, RefSpecifier::NON_REF));
                this->is_assignable = false;
                this->is_initialized = true;
            },
            [this](const CharExpr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::CHAR, RefSpecifier::NON_REF));
                this->is_assignable = false;
                this->is_initialized = true;
            },
            [this](const BoolExpr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::BOOL, RefSpecifier::NON_REF));
                this->is_assignable = false;
                this->is_initialized = true;
            },
            [this](const StringExpr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::STR, RefSpecifier::REF));
                this->is_assignable = false;
                this->is_initialized = true;
            },
            [this](const VariableExpr &node) {
                if (auto var = this->find_variable(node.name))
                {
                    this->last_type = std::make_unique<Type>(var->type);
                    this->is_assignable = var->mut;
                    this->is_initialized = var->initialized;
                }
                else
                {
                    this->report_error(L"referenced variable doesn't exist");
                    this->is_assignable = false;
                    this->last_type = nullptr;
                    this->is_initialized = false;
                }
            },
            //    [this](const BinaryExpr &node) { this->visit(node); },
            [this](const UnaryExpr &node) { this->visit(node); },
            //    [this](const CallExpr &node) { this->visit(node); },
            //    [this](const LambdaCallExpr &node) { this->visit(node); },
            //    [this](const IndexExpr &node) { this->visit(node); },
            [this](const CastExpr &node) { this->visit(node); },
            [](const auto &) {}},
        node);
}

void SemanticChecker::Visitor::visit(const Statement &node)
{
    std::visit(
        overloaded{
            [this](const Block &node) {
                for (const auto &stmt : node.statements)
                {
                    this->visit(*stmt);
                }
            },
            //    [this](const IfStmt &node) { this->visit(node); },
            //    [this](const WhileStmt &node) { this->visit(node); },
            //    [this](const MatchStmt &node) { this->visit(node); },
            //    [this](const ReturnStmt &node) { this->visit(node); },
            //    [this](const BreakStmt &node) { this->visit(node); },
            //    [this](const ContinueStmt &node) { this->visit(node); },
            [this](const FuncDefStmt &node) { this->visit(node); },
            [this](const AssignStmt &node) { this->visit(node); },
            [this](const ExprStmt &node) { this->visit(node); },
            [this](const VarDeclStmt &node) { this->visit(node); },
            //    [this](const ExternStmt &node) { this->visit(node); },
            [](const auto &) {}},
        node);
}

void SemanticChecker::Visitor::visit(const MatchArm &node)
{
}

void SemanticChecker::Visitor::visit(const Parameter &node)
{
}

void SemanticChecker::Visitor::visit(const Program &node)
{
    this->enter_scope();
    // for (auto &ext : node.externs)
    //     this->visit(*ext);
    for (auto &var : node.globals)
        this->visit(*var);
    for (auto &func : node.functions)
        this->visit(*func);
    this->leave_scope();
}

bool SemanticChecker::verify(const Program &program)
{
    this->visitor.visit(program);
    return this->visitor.value;
}

void SemanticChecker::add_logger(Logger *logger)
{
    this->visitor.add_logger(logger);
}

void SemanticChecker::remove_logger(Logger *logger)
{
    this->visitor.remove_logger(logger);
}
