#include "semantic_checker.hpp"
#include "ast.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <expected>
#include <optional>
#include <ranges>
#include <string_view>

namespace
{
std::unordered_map<TypeEnum, std::wstring> type_map = {
    {TypeEnum::BOOL, L"bool"}, {TypeEnum::U32, L"u32"},
    {TypeEnum::I32, L"i32"},   {TypeEnum::F64, L"f64"},
    {TypeEnum::CHAR, L"char"}, {TypeEnum::STR, L"str"},
};
std::unordered_map<RefSpecifier, std::wstring> ref_spec_map = {
    {RefSpecifier::NON_REF, L""},
    {RefSpecifier::REF, L"&"},
    {RefSpecifier::MUT_REF, L"&mut "},
};
std::unordered_map<UnaryOpEnum, std::unordered_map<TypeEnum, TypeEnum>>
    unary_map = {
        {UnaryOpEnum::INC,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
             {TypeEnum::F64, TypeEnum::F64},
         }},
        {UnaryOpEnum::DEC,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
             {TypeEnum::F64, TypeEnum::F64},
         }},
        {UnaryOpEnum::MINUS,
         {
             {TypeEnum::U32, TypeEnum::I32},
             {TypeEnum::I32, TypeEnum::I32},
             {TypeEnum::F64, TypeEnum::F64},
         }},
        {UnaryOpEnum::BIT_NEG,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
         }},
        {UnaryOpEnum::NEG,
         {
             {TypeEnum::BOOL, TypeEnum::BOOL},
         }},
};

std::unordered_map<
    BinOpEnum,
    std::unordered_map<TypeEnum, std::unordered_map<TypeEnum, TypeEnum>>>
    binary_map = {
        {BinOpEnum::ADD,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::I32}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::F64}}},
             {TypeEnum::STR,
              {{TypeEnum::STR, TypeEnum::STR},
               {TypeEnum::CHAR, TypeEnum::STR}}},
         }},
        {BinOpEnum::SUB,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::I32}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::F64}}},
         }},
        {BinOpEnum::MUL,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::I32}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::F64}}},
         }},
        {BinOpEnum::DIV,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::I32}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::F64}}},
         }},
        {BinOpEnum::MOD,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::I32}}},
         }},
        {BinOpEnum::EXP,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::U32, TypeEnum::I32}}},
             {TypeEnum::F64, {{TypeEnum::U32, TypeEnum::F64}}},
         }},
        {BinOpEnum::EQ,
         {
             {TypeEnum::BOOL, {{TypeEnum::BOOL, TypeEnum::BOOL}}},
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::BOOL}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::BOOL}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::BOOL}}},
             {TypeEnum::CHAR, {{TypeEnum::CHAR, TypeEnum::BOOL}}},
             {TypeEnum::STR, {{TypeEnum::STR, TypeEnum::BOOL}}},
         }},
        {BinOpEnum::NEQ,
         {
             {TypeEnum::BOOL, {{TypeEnum::BOOL, TypeEnum::BOOL}}},
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::BOOL}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::BOOL}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::BOOL}}},
             {TypeEnum::CHAR, {{TypeEnum::CHAR, TypeEnum::BOOL}}},
             {TypeEnum::STR, {{TypeEnum::STR, TypeEnum::BOOL}}},
         }},
        {BinOpEnum::GT,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::BOOL}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::BOOL}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::BOOL}}},
             {TypeEnum::CHAR, {{TypeEnum::CHAR, TypeEnum::BOOL}}},
         }},
        {BinOpEnum::GE,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::BOOL}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::BOOL}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::BOOL}}},
             {TypeEnum::CHAR, {{TypeEnum::CHAR, TypeEnum::BOOL}}},
         }},
        {BinOpEnum::LT,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::BOOL}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::BOOL}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::BOOL}}},
             {TypeEnum::CHAR, {{TypeEnum::CHAR, TypeEnum::BOOL}}},
         }},
        {BinOpEnum::LE,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::BOOL}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::BOOL}}},
             {TypeEnum::F64, {{TypeEnum::F64, TypeEnum::BOOL}}},
             {TypeEnum::CHAR, {{TypeEnum::CHAR, TypeEnum::BOOL}}},
         }},
        {BinOpEnum::BIT_AND,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::I32}}},
         }},
        {BinOpEnum::BIT_OR,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::I32}}},
         }},
        {BinOpEnum::BIT_XOR,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::I32, TypeEnum::I32}}},
         }},
        {BinOpEnum::SHL,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::U32, TypeEnum::I32}}},
         }},
        {BinOpEnum::SHR,
         {
             {TypeEnum::U32, {{TypeEnum::U32, TypeEnum::U32}}},
             {TypeEnum::I32, {{TypeEnum::U32, TypeEnum::I32}}},
         }},
        {BinOpEnum::AND,
         {
             {TypeEnum::BOOL, {{TypeEnum::BOOL, TypeEnum::BOOL}}},
         }},
        {BinOpEnum::OR,
         {
             {TypeEnum::BOOL, {{TypeEnum::BOOL, TypeEnum::BOOL}}},
         }},
};

std::unordered_map<UnaryOpEnum, std::wstring> unary_str_map = {
    {UnaryOpEnum::INC, L"incremented"},
    {UnaryOpEnum::DEC, L"decremented"},
    {UnaryOpEnum::MINUS, L"negated"},
    {UnaryOpEnum::NEG, L"negated logically"},
    {UnaryOpEnum::BIT_NEG, L"negated bitwise"},
};

constexpr std::wstring get_type_string(const Type &type)
{
    return std::visit(
        overloaded{[](const SimpleType &type) {
                       return ref_spec_map[type.ref_spec] +
                              type_map[type.type];
                   },
                   [](const FunctionType &type) {
                       std::wstring result = L"fn";
                       if (type.is_const)
                           result += L" const";
                       auto args_string =
                           type.arg_types |
                           std::views::transform(
                               [](const TypePtr &type_ptr) -> std::wstring {
                                   return get_type_string(*type_ptr);
                               }) |
                           std::views::join_with(L',');
                       result += L'(';
                       std::ranges::copy(std::ranges::cbegin(args_string),
                                         std::ranges::cend(args_string),
                                         std::back_inserter(result));
                       result += L')';
                       if (type.return_type)
                       {
                           result += L"=>";
                           result += get_type_string(*type.return_type);
                       }
                       return result;
                   }},
        type);
}

std::wstring get_type_string(const TypePtr &type_ptr)
{
    return (type_ptr) ? (get_type_string(*type_ptr)) : (L"void");
}
} // namespace

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
        this->report_error(L"wrong main function return type declaration - "
                           L"expected return type: void or u32, found: ",
                           get_type_string(node.return_type));
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
                this->report_error(L"variable of declared type: `",
                                   get_type_string(node.type),
                                   L"` cannot be assigned a value of type: `",
                                   get_type_string(value_type), L'`');
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

std::optional<FunctionType> SemanticChecker::Visitor::find_function(
    const std::wstring &name)
{
    for (const auto &scope : this->function_map)
    {
        auto found = scope.find(name);
        if (found != scope.end())
            return found->second;
    }
    return std::nullopt;
}

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

void SemanticChecker::Visitor::register_local_function(const FuncDefStmt &node)
{
    auto fn_type = node.get_type();
    this->function_map.back().emplace(
        std::make_pair(node.name, std::move(*fn_type)));
}

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

void SemanticChecker::Visitor::visit(const BinaryExpr &node)
{
    this->visit(*node.lhs);
    if (!this->last_type)
        return;
    auto left_type_var = *this->last_type;

    this->visit(*node.rhs);
    if (!this->last_type)
        return;
    auto right_type_var = *this->last_type;
    if (std::holds_alternative<FunctionType>(left_type_var) ||
        std::holds_alternative<FunctionType>(right_type_var))
    {
        this->report_expr_error(
            L"function reference cannot be used in a binary expression");
        return;
    }

    auto left_type = std::get<SimpleType>(left_type_var);
    auto right_type = std::get<SimpleType>(right_type_var);
    switch (left_type.ref_spec)
    {
    case RefSpecifier::REF:
        if (left_type.type != TypeEnum::STR)
        {
            this->report_expr_error(
                L"references cannot be used in binary expressions");
            return;
        }
    case RefSpecifier::NON_REF:
        break;

    default:
        this->report_expr_error(
            L"references cannot be used in binary expressions");
        return;
    }
    switch (right_type.ref_spec)
    {
    case RefSpecifier::REF:
        if (right_type.type != TypeEnum::STR)
        {
            this->report_expr_error(
                L"references cannot be used in binary expressions");
            return;
        }
    case RefSpecifier::NON_REF:
        break;

    default:
        this->report_expr_error(
            L"references cannot be used in binary expressions");
        return;
    }
    try
    {
        auto result_type =
            binary_map.at(node.op).at(left_type.type).at(right_type.type);
        if (result_type == TypeEnum::STR)
            this->last_type = std::make_unique<Type>(
                SimpleType(TypeEnum::STR, RefSpecifier::REF));
        else
            this->last_type = std::make_unique<Type>(
                SimpleType(result_type, RefSpecifier::NON_REF));
    }
    catch (const std::out_of_range &)
    {
        this->report_expr_error(L"binary operation doesn't support types `",
                                get_type_string(left_type_var), L"` and `",
                                get_type_string(right_type_var), L"`");
    }
}

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
    if (std::holds_alternative<FunctionType>(*this->last_type))
    {
        this->report_expr_error(
            L"function reference cannot be used in a unary expression");
        return;
    }
    auto type = std::get<SimpleType>(*this->last_type);
    switch (node.op)
    {
    case UnaryOpEnum::INC:
    case UnaryOpEnum::DEC:
    case UnaryOpEnum::MINUS:
    case UnaryOpEnum::BIT_NEG:
    case UnaryOpEnum::NEG:
        if (type.ref_spec != RefSpecifier::NON_REF)
        {
            this->report_expr_error(L"reference type value cannot be ",
                                    unary_str_map.at(node.op));
            return;
        }
        try
        {
            auto result_type = unary_map.at(node.op).at(type.type);
            this->last_type = std::make_unique<Type>(
                SimpleType(result_type, RefSpecifier::NON_REF));
        }
        catch (const std::out_of_range &)
        {
            this->report_expr_error(L"value of type `", get_type_string(type),
                                    L" cannot be ", unary_str_map.at(node.op));
        }

        /* code */
        break;
    case UnaryOpEnum::MUT_REF:
    case UnaryOpEnum::REF:

        if (type.ref_spec != RefSpecifier::NON_REF)
        {
            this->report_expr_error(
                L"references cannot be referenced further");
            break;
        }
        this->last_type = std::make_unique<Type>(
            SimpleType(type.type, get_ref_specifier(node.op)));
        this->is_assignable = false;
        break;
    case UnaryOpEnum::DEREF:
        if (type.type == TypeEnum::STR)
        {
            this->report_expr_error(L"strings cannot be dereferenced");
            break;
        }
        switch (type.ref_spec)
        {
        case RefSpecifier::NON_REF:
            this->report_error(L"cannot dereference a non-reference value");
            break;
        case RefSpecifier::REF:
            this->last_type = std::make_unique<Type>(
                SimpleType(type.type, RefSpecifier::NON_REF));
            this->is_assignable = false;
            break;
        case RefSpecifier::MUT_REF:
            this->last_type = std::make_unique<Type>(
                SimpleType(type.type, RefSpecifier::NON_REF));
            this->is_assignable = true;
            break;
        }
        break;
    }
}

void SemanticChecker::Visitor::visit(const CallExpr &node)
{
    this->visit(*node.callable);
    if (!this->last_type)
        return;

    if (std::holds_alternative<SimpleType>(*this->last_type))
    {
        this->report_expr_error(L"value of primitive type `",
                                get_type_string(*this->last_type),
                                L"` cannot be called");
        return;
    }
    auto type = std::get<FunctionType>(*this->last_type);
    auto expected_arg_count = type.arg_types.size();
    auto arg_count = node.args.size();
    if (arg_count != expected_arg_count)
    {
        this->report_expr_error(L"function argument count incorrect in a call "
                                L"expression: expected ",
                                expected_arg_count, L" arguments, found ",
                                arg_count);
    }
    auto is_valid = true;
    for (const auto &[expected_type, arg] :
         std::views::zip(type.arg_types, node.args))
    {
        this->visit(*arg);
        if (!this->last_type)
        {
            is_valid = false;
            continue;
        }
        auto actual_type = *this->last_type;
        if (*expected_type != actual_type)
        {
            this->report_expr_error(
                L"function call argument type mismatched - expected type: `",
                get_type_string(expected_type), "`, found: `",
                get_type_string(actual_type), "`");
            is_valid = false;
        }
    }
    if (is_valid)
    {
        this->last_type = clone_type_ptr(type.return_type);
    }
}

void SemanticChecker::Visitor::visit(const LambdaCallExpr &node)
{
    this->visit(*node.callable);
    if (!this->last_type)
        return;

    if (std::holds_alternative<SimpleType>(*this->last_type))
    {
        this->report_expr_error(L"value of primitive type `",
                                get_type_string(*this->last_type),
                                L"` cannot be called");
        return;
    }
    auto type = std::get<FunctionType>(*this->last_type);
    auto expected_arg_count = type.arg_types.size();
    auto arg_count = node.args.size();
    if (arg_count != expected_arg_count)
    {
        this->report_expr_error(
            L"function argument count incorrect in a lambda call "
            L"expression: expected ",
            expected_arg_count, L" arguments, found ", arg_count);
    }
    auto is_valid = true;
    auto is_const = type.is_const;
    std::vector<TypePtr> remaining_args;
    for (const auto &[expected_type, arg] :
         std::views::zip(type.arg_types, node.args))
    {
        if (arg)
        {
            this->visit(*arg);
            if (!this->last_type)
            {
                is_valid = false;
                continue;
            }
            auto actual_type = *this->last_type;
            if (*expected_type != actual_type)
            {
                this->report_expr_error(L"function call argument type "
                                        L"mismatched - expected type: `",
                                        get_type_string(expected_type),
                                        L"`, found: `",
                                        get_type_string(actual_type), L"`");
                is_valid = false;
            }
            if (std::holds_alternative<SimpleType>(actual_type))
            {
                auto simple = std::get<SimpleType>(actual_type);
                if (simple.ref_spec != RefSpecifier::NON_REF)
                {
                    is_const = false;
                }
            }
        }
        else
        {
            remaining_args.emplace_back(clone_type_ptr(expected_type));
        }
    }
    if (is_valid)
    {
        auto fn_type =
            FunctionType(std::move(remaining_args),
                         clone_type_ptr(type.return_type), is_const);
        this->last_type = std::make_unique<Type>(std::move(fn_type));
    }
}

void SemanticChecker::Visitor::visit(const IndexExpr &node)
{
    this->visit(*node.expr);
    if (!this->last_type)
    {
        return;
    }
    if (std::holds_alternative<FunctionType>(*this->last_type))
    {
        this->report_expr_error(L"function references cannot be indexed");
        return;
    }
    auto type = std::get<SimpleType>(*this->last_type);
    if (type != SimpleType(TypeEnum::STR, RefSpecifier::REF))
    {
        this->report_expr_error(
            L"value of type `", get_type_string(type),
            "` cannot be indexed (only `&str` values can be)");
        return;
    }
    this->visit(*node.index_value);
    if (std::holds_alternative<FunctionType>(*this->last_type))
    {
        this->report_expr_error(L"function references cannot be indices");
        return;
    }
    type = std::get<SimpleType>(*this->last_type);
    if (type != SimpleType(TypeEnum::U32, RefSpecifier::NON_REF))
    {
        this->report_expr_error(L"only `u32` values can be used as an index");
        return;
    }
    this->last_type = std::make_unique<Type>(
        SimpleType(TypeEnum::CHAR, RefSpecifier::NON_REF));
}

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

    this->register_local_function(node);
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
        overloaded{[this](const U32Expr &node) {
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
                       else if (auto func = this->find_function(node.name))
                       {
                           this->last_type = std::make_unique<Type>(*func);
                           this->is_assignable = false;
                           this->is_initialized = true;
                       }
                       else
                       {
                           this->report_expr_error(
                               L"referenced variable doesn't exist");
                           this->is_initialized = false;
                       }
                   },
                   [this](const BinaryExpr &node) { this->visit(node); },
                   [this](const UnaryExpr &node) { this->visit(node); },
                   [this](const CallExpr &node) { this->visit(node); },
                   [this](const LambdaCallExpr &node) { this->visit(node); },
                   [this](const IndexExpr &node) { this->visit(node); },
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
