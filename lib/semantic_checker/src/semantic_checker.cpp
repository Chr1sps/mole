#include "semantic_checker.hpp"
#include "ast.hpp"
#include "string_builder.hpp"
#include <algorithm>
#include <expected>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>

namespace
{
const std::unordered_map<TypeEnum, std::wstring> type_map = {
    {TypeEnum::BOOL, L"bool"}, {TypeEnum::U32, L"u32"},
    {TypeEnum::I32, L"i32"},   {TypeEnum::F64, L"f64"},
    {TypeEnum::CHAR, L"char"}, {TypeEnum::STR, L"str"},
};
const std::unordered_map<RefSpecifier, std::wstring> ref_spec_map = {
    {RefSpecifier::NON_REF, L""},
    {RefSpecifier::REF, L"&"},
    {RefSpecifier::MUT_REF, L"&mut "},
};
const std::unordered_map<UnaryOpEnum, std::unordered_map<TypeEnum, TypeEnum>>
    unary_map = {
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

const std::unordered_map<
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

const std::unordered_map<AssignType,
                         std::unordered_multimap<TypeEnum, TypeEnum>>
    assign_map = {
        {AssignType::NORMAL,
         {
             {TypeEnum::BOOL, TypeEnum::BOOL},
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
             {TypeEnum::F64, TypeEnum::F64},
             {TypeEnum::CHAR, TypeEnum::CHAR},
             {TypeEnum::STR, TypeEnum::STR},
         }},
        {AssignType::PLUS,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
             {TypeEnum::F64, TypeEnum::F64},
             {TypeEnum::STR, TypeEnum::CHAR},
             {TypeEnum::STR, TypeEnum::STR},
         }},
        {AssignType::MINUS,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
             {TypeEnum::F64, TypeEnum::F64},
         }},
        {AssignType::MUL,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
             {TypeEnum::F64, TypeEnum::F64},
         }},
        {AssignType::DIV,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
             {TypeEnum::F64, TypeEnum::F64},
         }},
        {AssignType::MOD,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
         }},
        {AssignType::EXP,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::U32},
             {TypeEnum::F64, TypeEnum::U32},
         }},
        {AssignType::BIT_AND,
         {
             {TypeEnum::BOOL, TypeEnum::BOOL},
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
         }},
        {AssignType::BIT_OR,
         {
             {TypeEnum::BOOL, TypeEnum::BOOL},
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
         }},
        {AssignType::BIT_XOR,
         {
             {TypeEnum::BOOL, TypeEnum::BOOL},
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::I32},
         }},
        {AssignType::SHL,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::U32},
         }},
        {AssignType::SHR,
         {
             {TypeEnum::U32, TypeEnum::U32},
             {TypeEnum::I32, TypeEnum::U32},
         }},
};

const std::unordered_map<UnaryOpEnum, std::wstring> unary_str_map = {
    {UnaryOpEnum::MINUS, L"negated"},
    {UnaryOpEnum::NEG, L"negated logically"},
    {UnaryOpEnum::BIT_NEG, L"negated bitwise"},
};

constexpr std::wstring get_type_string(const Type &type)
{
    return std::visit(
        overloaded{[](const SimpleType &type) {
                       return ref_spec_map.at(type.ref_spec) +
                              type_map.at(type.type);
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

constexpr bool check_non_ref_or_string(const Type &type)
{
    if (std::holds_alternative<FunctionType>(type))
        return false;
    auto simple = std::get<SimpleType>(type);
    switch (simple.ref_spec)
    {
    case RefSpecifier::REF:
        if (simple.type != TypeEnum::STR)
            return false;
        [[fallthrough]];
    case RefSpecifier::NON_REF:
        return true;

    default:
        return false;
    }
}
} // namespace

const std::unordered_multimap<TypeEnum, TypeEnum>
    SemanticChecker::Visitor::cast_map = {
        {TypeEnum::BOOL, TypeEnum::BOOL}, {TypeEnum::BOOL, TypeEnum::U32},
        {TypeEnum::BOOL, TypeEnum::I32},  {TypeEnum::BOOL, TypeEnum::F64},
        {TypeEnum::U32, TypeEnum::U32},   {TypeEnum::U32, TypeEnum::I32},
        {TypeEnum::U32, TypeEnum::F64},   {TypeEnum::U32, TypeEnum::CHAR},
        {TypeEnum::I32, TypeEnum::U32},   {TypeEnum::I32, TypeEnum::I32},
        {TypeEnum::I32, TypeEnum::F64},   {TypeEnum::F64, TypeEnum::U32},
        {TypeEnum::F64, TypeEnum::I32},   {TypeEnum::F64, TypeEnum::F64},
        {TypeEnum::CHAR, TypeEnum::U32},  {TypeEnum::CHAR, TypeEnum::I32},
        {TypeEnum::CHAR, TypeEnum::CHAR},
};

SemanticChecker::Visitor::Visitor() noexcept : value(true)
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

void SemanticChecker::Visitor::check_function_params(const FuncDefStmt &node)
{
    for (auto &param : node.params)
    {
        if (auto var = this->find_variable(param->name))
            this->report_error(param->position,
                               L"param name cannot shadow a variable that is "
                               L"already in scope");
        if (auto func = this->find_function(param->name))
            this->report_error(param->position,
                               L"param name cannot shadow a function that is "
                               L"already in scope");
    }
}

void SemanticChecker::Visitor::check_function_params(const ExternStmt &node)
{
    for (auto &param : node.params)
    {
        if (auto var = this->find_variable(param->name))
            this->report_error(node.position,
                               L"param name cannot shadow a variable that is "
                               L"already in scope");
        if (auto func = this->find_function(param->name))
            this->report_error(node.position,
                               L"param name cannot shadow a function that is "
                               L"already in scope");
    }
}

void SemanticChecker::Visitor::check_name_shadowing(const std::wstring &name,
                                                    const Position &pos)
{
    for (const auto &scope : this->variable_map)
    {
        auto iter = scope.find(name);
        if (iter != scope.cend())
            this->report_error(
                pos, L"given name is the same as that of another variable");
    }
    for (const auto &scope : this->function_map)
    {
        auto iter = scope.find(name);
        if (iter != scope.cend())
            this->report_error(
                pos, L"given name is the same as that of another function");
    }
}

void SemanticChecker::Visitor::check_main_function(const FuncDefStmt &node)
{
    if (!(!node.return_type ||
          *node.return_type ==
              SimpleType(TypeEnum::U32, RefSpecifier::NON_REF)))
    {
        this->report_error(node.position,
                           L"wrong main function return type declaration - "
                           L"expected return type: void or u32, found: ",
                           get_type_string(node.return_type));
    }
    if (!node.params.empty())
    {
        this->report_error(node.position, L"main cannot have any parameters");
    }
}

void SemanticChecker::Visitor::check_name_not_main(const VarDeclStmt &node)
{
    if (node.name == L"main")
        this->report_error(node.position, L"variable cannot be named 'main'");
}

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
                this->report_error(node.position,
                                   L"variable of declared type: `",
                                   get_type_string(node.type),
                                   L"` cannot be assigned a value of type: `",
                                   get_type_string(value_type), L'`');
                return false;
            }
        }
    }
    return true;
}

auto SemanticChecker::Visitor::find_variable(const std::wstring &name)
    -> std::optional<VarData>
{
    for (const auto &scope :
         std::span(this->variable_map.cbegin(), this->variable_map.cend() - 1))
    {
        auto found = scope.find(name);
        if (found != scope.cend())
        {
            this->is_local = false;
            return found->second;
        }
    }
    auto last_scope = this->variable_map.back();
    auto found = last_scope.find(name);
    if (found != last_scope.cend())
    {
        this->is_local = true;
        return found->second;
    }
    return std::nullopt;
}

std::optional<FunctionType> SemanticChecker::Visitor::find_function(
    const std::wstring &name) const
{
    for (const auto &scope : this->function_map)
    {
        auto found = scope.find(name);
        if (found != scope.end())
            return found->second;
    }
    return std::nullopt;
}

void SemanticChecker::Visitor::register_local_variable(const VarDeclStmt &node)
{
    auto type = (node.type) ? (*node.type) : (*this->last_type);
    auto new_data = VarData(type, node.is_mut);
    this->variable_map.back().emplace(std::make_pair(node.name, new_data));
}

void SemanticChecker::Visitor::register_local_function(const FuncDefStmt &node)
{
    auto fn_type = node.get_type();
    this->function_map.back().emplace(
        std::make_pair(node.name, std::move(*fn_type)));
}

void SemanticChecker::Visitor::register_local_function(const ExternStmt &node)
{
    auto fn_type = node.get_type();
    this->function_map.back().emplace(
        std::make_pair(node.name, std::move(*fn_type)));
}

void SemanticChecker::Visitor::register_function_params(
    const FuncDefStmt &node)
{
    for (auto &param : node.params)
    {
        this->visit(*param);
        auto type = *this->last_type;
        auto new_data = VarData(type, false);
        this->variable_map.back().emplace(
            std::make_pair(param->name, new_data));
    }
}

void SemanticChecker::Visitor::visit(const BinaryExpr &node)
{
    this->visit(*node.lhs);
    if (!this->last_type)
        return;
    auto left_type_var = *this->last_type;
    // auto is_left_const = this->is_const;

    this->visit(*node.rhs);
    if (!this->last_type)
        return;
    auto right_type_var = *this->last_type;
    // auto is_right_const = this->is_const;

    // this->is_const = is_left_const && is_right_const;
    if (!check_non_ref_or_string(left_type_var))
    {
        this->report_expr_error(
            get_position(*node.lhs),
            L"left hand side type cannot be used in a binary expression");
        return;
    }
    if (!check_non_ref_or_string(right_type_var))
    {
        this->report_expr_error(
            get_position(*node.rhs),
            L"right hand side type cannot be used in a binary expression");
        return;
    }
    auto left_type = std::get<SimpleType>(left_type_var);
    auto right_type = std::get<SimpleType>(right_type_var);

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
        this->report_expr_error(node.position,
                                L"binary operation doesn't support types `",
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
            get_position(*node.expr),
            L"function reference cannot be used in a unary expression");
        return;
    }

    auto type = std::get<SimpleType>(*this->last_type);
    switch (node.op)
    {
    case UnaryOpEnum::MINUS:
    case UnaryOpEnum::BIT_NEG:
    case UnaryOpEnum::NEG:
        if (type.ref_spec != RefSpecifier::NON_REF)
        {
            this->report_expr_error(node.position,
                                    L"reference type value cannot be ",
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
            this->report_expr_error(node.position, L"value of type `",
                                    get_type_string(type), L" cannot be ",
                                    unary_str_map.at(node.op));
        }

        /* code */
        break;
    case UnaryOpEnum::MUT_REF:
    case UnaryOpEnum::REF:

        if (type.ref_spec != RefSpecifier::NON_REF)
        {
            this->report_expr_error(
                node.position, L"references cannot be referenced further");
            break;
        }
        if (this->ref_spec == RefSpecifier::NON_REF)
        {
            this->report_expr_error(node.position,
                                    L"referenced values must be variables");
            break;
        }
        this->last_type = std::make_unique<Type>(
            SimpleType(type.type, get_ref_specifier(node.op)));
        this->ref_spec = RefSpecifier::NON_REF;
        break;
    case UnaryOpEnum::DEREF:
        if (type.type == TypeEnum::STR)
        {
            this->report_expr_error(node.position,
                                    L"strings cannot be dereferenced");
            break;
        }
        switch (type.ref_spec)
        {
        case RefSpecifier::NON_REF:
            this->report_error(node.position,
                               L"cannot dereference a non-reference value");
            break;
        case RefSpecifier::REF:
        case RefSpecifier::MUT_REF:
            this->last_type = std::make_unique<Type>(
                SimpleType(type.type, RefSpecifier::NON_REF));
            this->ref_spec = type.ref_spec;
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
        this->report_expr_error(
            get_position(*node.callable), L"value of primitive type `",
            get_type_string(*this->last_type), L"` cannot be called");
        return;
    }

    auto type = std::get<FunctionType>(*this->last_type);
    // auto is_const = type.is_const;
    auto expected_arg_count = type.arg_types.size();
    auto arg_count = node.args.size();
    if (arg_count != expected_arg_count)
    {
        this->report_expr_error(node.position,
                                L"function argument count incorrect in a call "
                                L"expression: expected ",
                                expected_arg_count, L" arguments, found ",
                                arg_count);
    }
    auto is_valid = true;
    for (const auto &[expected_type, arg] :
         std::views::zip(type.arg_types, node.args))
    {
        this->visit(*arg);
        // is_const &= this->is_const;
        if (!this->last_type)
        {
            is_valid = false;
            continue;
        }
        auto actual_type = *this->last_type;
        if (*expected_type != actual_type)
        {
            this->report_expr_error(
                get_position(*arg),
                L"function call argument type mismatched - expected type: `",
                get_type_string(expected_type), "`, found: `",
                get_type_string(actual_type), "`");
            is_valid = false;
        }
    }
    if (is_valid)
    {
        this->last_type = clone_type_ptr(type.return_type);
        // this->is_const = is_const;
    }
}

void SemanticChecker::Visitor::visit(const LambdaCallExpr &node)
{
    this->visit(*node.callable);
    if (!this->last_type)
        return;

    if (std::holds_alternative<SimpleType>(*this->last_type))
    {
        this->report_expr_error(
            get_position(*node.callable), L"value of primitive type `",
            get_type_string(*this->last_type), L"` cannot be called");
        return;
    }
    auto type = std::get<FunctionType>(*this->last_type);
    auto expected_arg_count = type.arg_types.size();
    auto arg_count = node.args.size();
    if (arg_count != expected_arg_count)
    {
        this->report_expr_error(
            node.position,
            L"function argument count incorrect in a lambda call "
            L"expression: expected ",
            expected_arg_count, L" arguments, found ", arg_count);
    }
    auto is_valid = true;
    auto is_const = type.is_const;
    // auto is_compile_time = type.is_const;
    std::vector<TypePtr> remaining_args;
    for (const auto &[expected_type, arg] :
         std::views::zip(type.arg_types, node.args))
    {
        if (arg)
        {
            this->visit(*arg);
            // is_compile_time &= this->is_const;
            if (!this->last_type)
            {
                is_valid = false;
                continue;
            }
            auto actual_type = *this->last_type;
            if (*expected_type != actual_type)
            {
                this->report_expr_error(
                    ((arg) ? (get_position(*arg)) : (node.position)),
                    L"function call argument type "
                    L"mismatched - expected type: `",
                    get_type_string(expected_type), L"`, found: `",
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
        // this->is_const = is_compile_time;
    }
}

void SemanticChecker::Visitor::visit(const IndexExpr &node)
{
    this->visit(*node.expr);
    // auto is_const = this->is_const;
    if (!this->last_type)
    {
        return;
    }
    if (std::holds_alternative<FunctionType>(*this->last_type))
    {
        this->report_expr_error(get_position(*node.expr),
                                L"function references cannot be indexed");
        return;
    }
    auto type = std::get<SimpleType>(*this->last_type);
    if (type != SimpleType(TypeEnum::STR, RefSpecifier::REF))
    {
        this->report_expr_error(
            get_position(*node.expr), L"value of type `",
            get_type_string(type),
            "` cannot be indexed (only `&str` values can be)");
        return;
    }
    this->visit(*node.index_value);
    // is_const &= this->is_const;
    if (std::holds_alternative<FunctionType>(*this->last_type))
    {
        this->report_expr_error(get_position(*node.index_value),
                                L"function references cannot be indices");
        return;
    }
    type = std::get<SimpleType>(*this->last_type);
    if (type != SimpleType(TypeEnum::U32, RefSpecifier::NON_REF))
    {
        this->report_expr_error(get_position(*node.index_value),
                                L"only `u32` values can be used as an index");
        return;
    }
    this->last_type = std::make_unique<Type>(
        SimpleType(TypeEnum::CHAR, RefSpecifier::NON_REF));
    // this->is_const = is_const;
}

void SemanticChecker::Visitor::visit(const CastExpr &node)
{
    this->visit(*node.expr);
    auto to_type = std::get<SimpleType>((*node.type));
    std::visit(
        overloaded{
            [this, &node, &to_type](const SimpleType &from_type) {
                if (from_type.ref_spec != RefSpecifier::NON_REF)
                {
                    this->report_error(node.position,
                                       L"cannot cast from a reference value");
                }
                if (to_type.ref_spec != RefSpecifier::NON_REF)
                {
                    this->report_error(node.position,
                                       L"cannot cast to a reference type");
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
                        node.position,
                        L"cast between two types not supported");
                    this->last_type = nullptr;
                }
                else
                {
                    this->last_type = std::make_unique<Type>(to_type);
                }
            },
            [this, &node](const FunctionType &) {
                this->report_error(node.position,
                                   L"function references cannot be cast");
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

void SemanticChecker::Visitor::visit(const AssignStmt &node)
{
    this->visit(*node.lhs);
    if (!this->last_type)
        return;
    if (this->ref_spec != RefSpecifier::MUT_REF)
    {
        this->report_error(
            get_position(*node.lhs),
            L"left side of the assignment statement is non-assignable");
        return;
    }
    auto left_type_var = *this->last_type;

    this->visit(*node.rhs);
    if (!this->last_type)
        return;
    auto right_type_var = *this->last_type;

    if (!check_non_ref_or_string(left_type_var))
    {
        this->report_expr_error(
            get_position(*node.lhs),
            L"left hand side type cannot be used in an assignment");
        return;
    }
    if (!check_non_ref_or_string(right_type_var))
    {
        this->report_expr_error(
            get_position(*node.rhs),
            L"right hand side type cannot be used in an assignment");
        return;
    }
    auto left_type = std::get<SimpleType>(left_type_var);
    auto right_type = std::get<SimpleType>(right_type_var);
    auto op_map = assign_map.at(node.type);
    decltype(op_map)::value_type type_pair =
        std::make_pair(left_type.type, right_type.type);
    auto iter = std::find(op_map.cbegin(), op_map.cend(), type_pair);
    if (iter == op_map.cend())
    {
        this->report_error(node.position, L"value of type `",
                           get_type_string(right_type),
                           L"` cannot be assigned to a value of type `",
                           get_type_string(left_type), L"` ");
    }
}

void SemanticChecker::Visitor::check_condition_expr(
    const Expression &condition)
{
    this->visit(condition);
    if (!this->last_type)
        return;
    if (*this->last_type != SimpleType(TypeEnum::BOOL, RefSpecifier::NON_REF))
    {
        this->report_error(
            get_position(condition),
            L"expected type `bool` in a condition expression, found `",
            get_type_string(this->last_type), L"`");
    }
}

void SemanticChecker::Visitor::visit(const IfStmt &node)
{
    auto prev_return_covered = this->is_return_covered;
    this->check_condition_expr(*node.condition_expr);
    this->visit(*node.then_block);
    auto is_then_return_covered = this->is_return_covered;
    if (node.else_block)
    {
        this->visit(*node.else_block);
        this->is_return_covered &= is_then_return_covered;
        this->is_return_covered |= prev_return_covered;
    }
    else
        this->is_return_covered = prev_return_covered;
}

void SemanticChecker::Visitor::visit(const WhileStmt &node)
{
    this->check_condition_expr(*node.condition_expr);
    auto previous_is_in_loop = std::exchange(this->is_in_loop, true);
    this->visit(*node.statement);
    this->is_in_loop = previous_is_in_loop;
    this->is_return_covered = false;
}

void SemanticChecker::Visitor::visit(const MatchStmt &node)
{
    this->visit(*node.matched_expr);
    if (!this->last_type)
        return;
    if (!check_non_ref_or_string(*this->last_type))
    {
        this->report_error(get_position(*node.matched_expr),
                           L"cannot match a value of type `",
                           get_type_string(this->last_type), "`");
    }
    auto previous_matched_type =
        std::exchange(this->matched_type, clone_type_ptr(this->last_type));
    auto previous_is_exhaustive = std::exchange(this->is_exhaustive, false);
    for (const auto &arm : node.match_arms)
    {
        if (this->is_exhaustive)
        {
            this->report_warning(get_position(*arm),
                                 L"this arm will not be reached");
        }
        this->visit(*arm);
    }
    if (!this->is_exhaustive)
    {
        this->report_warning(node.position,
                             L"match statement is not exhaustive");
    }
    this->matched_type = std::move(previous_matched_type);
    this->is_exhaustive = previous_is_exhaustive;
}

void SemanticChecker::Visitor::visit(const ReturnStmt &node)
{
    TypePtr return_type = nullptr;
    if (node.expr)
    {
        this->visit(*node.expr);
        return_type = clone_type_ptr(this->last_type);
    }
    if (!((!return_type && !this->expected_return_type) ||
          (return_type && this->expected_return_type &&
           *return_type == *this->expected_return_type)))
    {
        this->report_error(
            ((node.expr) ? (get_position(*node.expr)) : (node.position)),
            L"expected `", get_type_string(this->expected_return_type),
            L"` expression type in a return statement, found `",
            get_type_string(return_type), L"`");
    }
    this->is_return_covered = true;
}

void SemanticChecker::Visitor::enter_function_scope(const bool &is_const)
{
    this->enter_scope();
    this->const_scopes.push_back(is_const);
    this->is_return_covered = false;
}

void SemanticChecker::Visitor::leave_function_scope()
{
    this->const_scopes.pop_back();
    this->leave_scope();
}

bool SemanticChecker::Visitor::is_in_const_scope() const
{
    auto iter = std::find(this->const_scopes.cbegin(),
                          this->const_scopes.cend(), true);
    return iter != this->const_scopes.cend();
}

void SemanticChecker::Visitor::visit(const FuncDefStmt &node)
{
    this->check_name_shadowing(node.name, node.position);
    if (node.name == L"main")
    {
        this->check_main_function(node);
    }

    this->check_function_params(node);

    this->enter_function_scope(node.is_const);
    this->register_function_params(node);

    auto previous_return_type = std::exchange(
        this->expected_return_type, clone_type_ptr(node.return_type));

    for (const auto &stmt : node.block->statements)
    {
        this->visit(*stmt);
    }

    if (!this->expected_return_type)
        this->is_return_covered = true;

    this->leave_function_scope();
    if (!this->is_return_covered)
    {
        this->report_error(
            node.position,
            L"function doesn't return in each control flow path");
    }

    this->expected_return_type = std::move(previous_return_type);

    this->register_local_function(node);
}

void SemanticChecker::Visitor::visit(const ExprStmt &node)
{
    this->visit(*node.expr);
}

void SemanticChecker::Visitor::visit(const VarDeclStmt &node)
{
    this->check_name_shadowing(node.name, node.position);
    auto registerable = true;
    if (node.is_mut)
    {
        if (!(node.initial_value || node.type))
        {
            this->report_error(node.position,
                               L"mutable must have either a type or a "
                               L"value assigned to it");
            registerable = false;
        }
    }
    else if (!node.initial_value)
    {
        this->report_error(node.position,
                           L"constant must have a value assigned to it");
        if (!node.type)
            registerable = false;
    }
    registerable &= this->check_var_value_and_type(node);
    this->check_name_not_main(node);
    if (registerable)
        this->register_local_variable(node);
}

void SemanticChecker::Visitor::visit(const ExternStmt &node)
{
    this->check_name_shadowing(node.name, node.position);
    if (node.name == L"main")
    {
        this->report_error(node.position, L"`main` cannot be externed");
    }

    this->check_function_params(node);

    this->register_local_function(node);
}

void SemanticChecker::Visitor::visit(const Expression &node)
{
    std::visit(
        overloaded{
            [this](const U32Expr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::U32, RefSpecifier::NON_REF));
                this->ref_spec = RefSpecifier::NON_REF;
                // this->is_const = true;
            },
            [this](const F64Expr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::F64, RefSpecifier::NON_REF));
                this->ref_spec = RefSpecifier::NON_REF;
                // this->is_const = true;
            },
            [this](const CharExpr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::CHAR, RefSpecifier::NON_REF));
                this->ref_spec = RefSpecifier::NON_REF;
                // this->is_const = true;
            },
            [this](const BoolExpr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::BOOL, RefSpecifier::NON_REF));
                this->ref_spec = RefSpecifier::NON_REF;
                // this->is_const = true;
            },
            [this](const StringExpr &node) {
                this->last_type = std::make_unique<Type>(
                    SimpleType(TypeEnum::STR, RefSpecifier::REF));
                this->ref_spec = RefSpecifier::NON_REF;
                // this->is_const = true;
            },
            [this](const VariableExpr &node) {
                if (auto var = this->find_variable(node.name))
                {
                    this->last_type = std::make_unique<Type>(var->type);
                    this->ref_spec = (var->mut) ? (RefSpecifier::MUT_REF)
                                                : (RefSpecifier::REF);
                    // this->is_const = !var->mut;
                    if (this->is_in_const_scope() && !this->is_local)
                    {
                        this->report_error(
                            node.position,
                            L"non-constant outside variable accessed in a "
                            L"constant function body");
                    }
                }
                else if (auto func = this->find_function(node.name))
                {
                    this->last_type = std::make_unique<Type>(*func);
                    this->ref_spec = RefSpecifier::REF;
                    // this->is_const = true;
                }
                else
                    this->report_expr_error(
                        node.position, L"referenced variable doesn't exist");
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
    if (this->is_return_covered)
        this->report_warning(get_position(node),
                             L"this statement will not execute - "
                             L"it is after a return statement");

    std::visit(
        overloaded{
            [this](const Block &node) {
                for (const auto &stmt : node.statements)
                {
                    this->visit(*stmt);
                }
                if (!this->expected_return_type)
                {
                    this->is_return_covered = true;
                }
            },
            [this](const IfStmt &node) { this->visit(node); },
            [this](const WhileStmt &node) { this->visit(node); },
            [this](const MatchStmt &node) { this->visit(node); },
            [this](const ReturnStmt &node) { this->visit(node); },
            [this](const BreakStmt &node) {
                if (!this->is_in_loop)
                {
                    this->report_error(
                        node.position,
                        L"break statement can only be used in a loop");
                }
            },
            [this](const ContinueStmt &node) {
                if (!this->is_in_loop)
                {
                    this->report_error(
                        node.position,
                        L"continue statement can only be used in a loop");
                }
            },
            [this](const FuncDefStmt &node) { this->visit(node); },
            [this](const AssignStmt &node) { this->visit(node); },
            [this](const ExprStmt &node) { this->visit(node); },
            [this](const VarDeclStmt &node) { this->visit(node); },
            [this](const ExternStmt &node) { this->visit(node); },
            [](const auto &) {}},
        node);
}

void SemanticChecker::Visitor::visit(const LiteralArm &node)
{
    for (const auto &literal : node.literals)
    {
        this->visit(*literal);
        if (!this->last_type)
            continue;
        if (*this->last_type != *this->matched_type)
        {
            this->report_error(
                get_position(*literal), L"literal of type `",
                get_type_string(*this->last_type),
                L"` cannot be matched against an expression of type `",
                get_type_string(*this->matched_type), L"`");
        }
    }
    this->visit(*node.block);
}

void SemanticChecker::Visitor::visit(const GuardArm &node)
{
    this->check_condition_expr(*node.condition_expr);
    this->visit(*node.block);
}

void SemanticChecker::Visitor::visit(const ElseArm &node)
{
    this->is_exhaustive = true;
    this->visit(*node.block);
}

void SemanticChecker::Visitor::visit(const MatchArm &node)
{
    std::visit(
        overloaded{[this](const LiteralArm &node) { this->visit(node); },
                   [this](const GuardArm &node) { this->visit(node); },
                   [this](const ElseArm &node) { this->visit(node); }},
        node);
}

void SemanticChecker::Visitor::visit(const Parameter &node)
{
    this->last_type = clone_type_ptr(node.type);
}

void SemanticChecker::Visitor::register_top_level(const FuncDefStmt &node)
{
    this->check_name_shadowing(node.name, node.position);
    if (node.name == L"main")
    {
        this->check_main_function(node);
    }

    this->check_function_params(node);

    this->register_local_function(node);
}

void SemanticChecker::Visitor::visit_top_level(const FuncDefStmt &node)
{
    this->enter_function_scope(node.is_const);
    this->register_function_params(node);

    this->expected_return_type = clone_type_ptr(node.return_type);

    for (const auto &stmt : node.block->statements)
    {
        this->visit(*stmt);
    }

    if (!this->expected_return_type)
        this->is_return_covered = true;

    this->leave_function_scope();
    if (!this->is_return_covered)
    {
        this->report_error(
            node.position,
            L"function doesn't return in each control flow path");
    }
}

void SemanticChecker::Visitor::visit(const Program &node)
{
    this->enter_scope();
    for (auto &ext : node.externs)
        this->visit(*ext);
    for (auto &var : node.globals)
        this->visit(*var);

    for (auto &func : node.functions)
        this->register_top_level(*func);
    for (auto &func : node.functions)
        this->visit_top_level(*func);
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
