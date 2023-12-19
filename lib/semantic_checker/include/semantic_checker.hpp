#ifndef __SEMANTIC_CHECKER_HPP__
#define __SEMANTIC_CHECKER_HPP__
#include "string_builder.hpp"
#include "types.hpp"
#include "visitor.hpp"
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>

class SemanticChecker : public AstVisitor
{
    struct Variable
    {
        std::wstring name;
        TypePtr type;
        bool mut;
        bool initialized;

        Variable(const std::wstring &name, const TypePtr &type,
                 const bool &mut, const bool &initialized)
            : name(name), type(type), mut(mut), initialized(initialized)
        {
        }
    };

    struct Function
    {
        std::wstring name;
        std::shared_ptr<FunctionType> type;

        Function(const std::wstring &name,
                 const std::shared_ptr<FunctionType> &type)
            : name(name), type(type)
        {
        }
    };

    TypePtr last_type, return_type;
    std::vector<Logger *> loggers;

    std::vector<std::unordered_set<std::shared_ptr<Variable>>> variables;
    std::vector<std::unordered_set<std::shared_ptr<Function>>> functions;
    std::vector<TypePtr> return_stack;

    std::vector<size_t> const_scopes;

    unsigned int scope_level = 0;
    bool check_const;

    void enter_scope();
    void leave_scope();
    void register_globals(const Program &node);

    void check_function_names(const VarDeclStmt &node);
    void check_function_params(const FuncDefStmt &node);
    void check_function_params(const ExternStmt &node);
    void check_function_block(const FuncDefStmt &node);
    void check_variable_names(const VarDeclStmt &node);
    void check_main_function(const FuncDefStmt &node);

    void check_var_value_and_type(const VarDeclStmt &node);

    void check_function_return(const FuncDefStmt &node);

    std::shared_ptr<Variable> find_variable(const std::wstring &name);
    std::shared_ptr<Function> find_function(const std::wstring &name);

    std::shared_ptr<Variable> find_outside_variable(const std::wstring &name);
    std::shared_ptr<Variable> find_outside_variable(const std::wstring &name,
                                                    const int &scope_number);
    std::shared_ptr<Function> find_outside_function(const std::wstring &name);

    void register_local_variable(const VarDeclStmt &node);
    void register_local_function(const FuncDefStmt &node);
    void register_local_function(const ExternStmt &node);

    void register_function_params(const FuncDefStmt &node);
    void unregister_function_params(const FuncDefStmt &node);

    void check_var_name(const VarDeclStmt &node);

    // template <typename... Args> void report_error(Args &&...args);
    void report_error(const std::wstring &error_msg);

  public:
    void visit(const VariableExpr &node) override; // tu trzeba dać
    void visit(const I32Expr &node) override;
    void visit(const F64Expr &node) override;
    void visit(const BinaryExpr &node) override;
    void visit(const UnaryExpr &node) override;
    void visit(const CallExpr &node) override;
    void visit(const LambdaCallExpr &node) override;
    void visit(const Block &node) override;
    void visit(const ReturnStmt &node) override;
    void visit(const FuncDefStmt &node) override;
    void visit(const AssignStmt &node) override; // tu trzeba dać
    void visit(const VarDeclStmt &node) override;
    void visit(const ExternStmt &node) override;
    void visit(const Program &node) override;
};

// template <typename... Args> void SemanticChecker::report_error(Args
// &&...args)
// {
//     std::wstring full_msg = build_wstring(L"Semantic error: ", args...,
//     L"."); throw SemanticException(full_msg);
// }
#endif