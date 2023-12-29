#ifndef __SEMANTIC_CHECKER_HPP__
#define __SEMANTIC_CHECKER_HPP__
#include "logger.hpp"
#include "string_builder.hpp"
#include "visitor.hpp"
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>

class SemanticChecker
{
    class Visitor : public AstVisitor, public Reporter
    {
        TypePtr last_type, return_type;

        template <typename... Args>
        void report_and_set_false(const LogLevel &log_level, Args &&...data)
        {
            this->report(log_level, data...);
            this->value = false;
        }

        // struct Variable
        // {
        //     std::wstring name;
        //     TypePtr type;
        //     bool mut;
        //     bool initialized;

        //     Variable(const std::wstring &name, const TypePtr &type,
        //              const bool &mut, const bool &initialized)
        //         : name(name), type(clone_type_ptr(type)), mut(mut),
        //           initialized(initialized)
        //     {
        //     }
        // };

        // struct Function
        // {
        //     std::wstring name;
        //     FunctionType type;

        //     Function(const std::wstring &name, const FunctionType &type)
        //         : name(name), type(type)
        //     {
        //     }
        // };

        // std::vector<std::unordered_set<std::shared_ptr<Variable>>>
        // variables;
        // std::vector<std::unordered_set<std::shared_ptr<Function>>>
        // functions; std::vector<TypePtr> return_stack;

        // std::vector<size_t> const_scopes;

        // unsigned int scope_level = 0;
        // bool check_const;

        // void enter_scope();
        // void leave_scope();
        // void register_globals(const Program &node);

        // void check_function_names(const VarDeclStmt &node);
        // void check_function_params(const FuncDefStmt &node);
        // void check_function_params(const ExternStmt &node);
        // void check_function_block(const FuncDefStmt &node);
        // void check_variable_names(const VarDeclStmt &node);
        // void check_main_function(const FuncDefStmt &node);

        void check_var_value_and_type(const VarDeclStmt &node);

        // void check_function_return(const FuncDefStmt &node);

        // std::shared_ptr<Variable> find_variable(const std::wstring &name);
        // std::shared_ptr<Function> find_function(const std::wstring &name);

        // std::shared_ptr<Variable> find_outside_variable(
        //     const std::wstring &name);
        // std::shared_ptr<Variable> find_outside_variable(
        //     const std::wstring &name, const int &scope_number);
        // std::shared_ptr<Function> find_outside_function(
        //     const std::wstring &name);

        // void register_local_variable(const VarDeclStmt &node);
        // void register_local_function(const FuncDefStmt &node);
        // void register_local_function(const ExternStmt &node);

        // void register_function_params(const FuncDefStmt &node);
        // void unregister_function_params(const FuncDefStmt &node);

        // void check_var_name(const VarDeclStmt &node);

        // void visit(const VariableExpr &node);
        // void visit(const U32Expr &node);
        // void visit(const F64Expr &node);
        // void visit(const BinaryExpr &node);
        // void visit(const UnaryExpr &node);
        // void visit(const CallExpr &node);
        // void visit(const LambdaCallExpr &node);

        // void visit(const Block &node);
        // void visit(const ReturnStmt &node);
        // void visit(const AssignStmt &node);
        // void visit(const FuncDefStmt &node);
        void visit(const VarDeclStmt &node);
        // void visit(const ExternStmt &node);

      public:
        Visitor() noexcept;
        void visit(const Statement &node) override;
        void visit(const ExprNode &node) override;
        void visit(const Type &node) override;
        void visit(const MatchArm &node) override;
        void visit(const Parameter &node) override;
        void visit(const Program &node) override;
        bool value;
    } visitor;

  public:
    void add_logger(Logger *logger);
    void remove_logger(Logger *logger);
    bool verify(const Program &program);
};
#endif