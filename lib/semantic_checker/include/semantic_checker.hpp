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
    class Visitor : public ExprVisitor,
                    public StmtVisitor,
                    public MatchArmVisitor,
                    public ParamVisitor,
                    public ProgramVisitor,
                    public Reporter
    {
        static const std::unordered_multimap<TypeEnum, TypeEnum> cast_map;
        std::optional<Type> last_type, expected_return_type, matched_type;
        bool is_in_loop, is_exhaustive, is_return_covered, is_local;
        RefSpecifier ref_spec;

        template <typename... Args>
        void report_error(const Position &pos, Args &&...data);

        template <typename... Args>
        void report_warning(const Position &pos, Args &&...data);

        template <typename... Args>
        void report_expr_error(const Position &pos, Args &&...data);

        struct VarData
        {
            Type type;
            bool mut;

            VarData(const Type &type, const bool &mut) : type(type), mut(mut)
            {
            }

            VarData(const VarData &other) : type(other.type), mut(other.mut)
            {
            }

            VarData &operator=(const VarData &other)
            {
                this->type = other.type;
                this->mut = other.mut;
                return *this;
            }
        };

        struct Function
        {
            std::vector<Type> param_types;
            std::optional<Type> return_type;
        };

        std::vector<std::unordered_map<std::wstring, VarData>> variable_map;
        std::vector<std::unordered_map<std::wstring, Function>> function_map;
        std::deque<bool> const_scopes;

        void enter_scope();
        void leave_scope();

        void check_function_params(const FuncDef &node);
        void check_function_params(const ExternDef &node);
        void check_main_function(const FuncDef &node);

        bool check_var_value_and_type(const VarDeclStmt &node);

        std::optional<VarData> find_variable(const std::wstring &name);
        std::optional<Function> find_function(const std::wstring &name) const;

        void register_local_function(const FuncDef &node);
        void register_local_function(const ExternDef &node);

        void register_function_params(const FuncDef &node);

        void check_condition_expr(const Expression &condition);

        void visit(const BinaryExpr &node);
        void visit(const UnaryExpr &node);
        void visit_call(const CallExpr &node);
        void visit(const IndexExpr &node);
        void visit(const CastExpr &node);

        void visit_block(const Block &node);
        void visit(const IfStmt &node);
        void visit(const WhileStmt &node);
        void visit(const MatchStmt &node);
        void visit(const ReturnStmt &node);
        void visit(const AssignStmt &node);

        void enter_function_scope(const bool &is_const);
        void leave_function_scope();
        bool is_in_const_scope() const;

        void visit(const FuncDef &node);
        void visit(const ExternDef &node);

        void check_name_shadowing(const std::wstring &name,
                                  const Position &pos);
        void check_name_not_main(const VarDeclStmt &node);
        void register_local_variable(const VarDeclStmt &node);

        void visit(const VarDeclStmt &node);

        void visit_top_level(const FuncDef &node);
        void register_top_level(const FuncDef &node);

        void visit(const LiteralArm &node);
        void visit(const GuardArm &node);
        void visit(const ElseArm &node);

      public:
        Visitor() noexcept;
        void visit(const Statement &node) override;
        void visit(const Expression &node) override;
        void visit(const MatchArm &node) override;
        void visit(const Parameter &node) override;
        void visit(const Program &node) override;
        bool value;
    } visitor;

  public:
    void add_logger(Logger *logger);
    void remove_logger(Logger *logger);
    void check(const Program &program);
};

template <typename... Args>
void SemanticChecker::Visitor::report_error(const Position &pos,
                                            Args &&...data)
{
    this->report(LogLevel::ERROR, "Semantic error at [", pos.line, ",",
                 pos.column, "]: ", data..., ".");
    this->value = false;
}

template <typename... Args>
void SemanticChecker::Visitor::report_warning(const Position &pos,
                                              Args &&...data)
{
    this->report(LogLevel::WARNING, "Semantic warning at [", pos.line, ",",
                 pos.column, "]: ", data..., ".");
}

template <typename... Args>
void SemanticChecker::Visitor::report_expr_error(const Position &pos,
                                                 Args &&...data)
{
    this->report_error(pos, data...);
    this->last_type = std::nullopt;
    this->ref_spec = RefSpecifier::NON_REF;
}
#endif