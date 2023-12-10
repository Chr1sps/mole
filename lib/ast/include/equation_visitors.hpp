#include "ast.hpp"
#include "visitor.hpp"

#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...)                                                  \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...)                                       \
    macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define FALSE_VISIT(type)                                                     \
    void visit(const type &other) override                                    \
    {                                                                         \
        value = false;                                                        \
    }
#define GENERATE_FALSE_VISITS(...) FOR_EACH(FALSE_VISIT, __VA_ARGS__)

#define GENERATE_VISITOR(base, type, visit_body, ...)                         \
    struct type##Visitor : base                                               \
    {                                                                         \
        bool value;                                                           \
        type##Visitor(const type &expr) : expr(expr)                          \
        {                                                                     \
        }                                                                     \
        GENERATE_FALSE_VISITS(__VA_ARGS__)                                    \
        void visit(const type &node) override                                 \
                                                                              \
            visit_body                                                        \
                                                                              \
            private : const type &expr;                                       \
    };

template <typename T>
bool equal_or_null(const std::unique_ptr<T> &first,
                   const std::unique_ptr<T> &other)
{
    return (first == nullptr && other == nullptr) ||
           (first && other && *first == *other);
}

template <typename T>
bool equal_or_null(const std::shared_ptr<T> &first,
                   const std::shared_ptr<T> &other)
{
    return (first == nullptr && other == nullptr) ||
           (first && other && *first == *other);
}

template <typename T> bool equal_or_null(const T *first, const T *other)
{
    return (first == nullptr && other == nullptr) ||
           (first && other && *first == *other);
}

GENERATE_VISITOR(
    ExprVisitor, VariableExpr,
    {
        this->value = this->expr.name == node.name &&
                      this->expr.position == node.position;
    },
    I32Expr, F64Expr, StringExpr, CharExpr, BoolExpr, BinaryExpr, UnaryExpr,
    CallExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, I32Expr,
    {
        this->value = this->expr.value == node.value &&
                      this->expr.position == node.position;
    },
    VariableExpr, F64Expr, StringExpr, CharExpr, BoolExpr, BinaryExpr,
    UnaryExpr, CallExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, F64Expr,
    {
        this->value = this->expr.value == node.value &&
                      this->expr.position == node.position;
    },
    VariableExpr, I32Expr, StringExpr, CharExpr, BoolExpr, BinaryExpr,
    UnaryExpr, CallExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, StringExpr,
    {
        this->value = this->expr.value == node.value &&
                      this->expr.position == node.position;
    },
    VariableExpr, I32Expr, F64Expr, CharExpr, BoolExpr, BinaryExpr, UnaryExpr,
    CallExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, CharExpr,
    {
        this->value = this->expr.value == node.value &&
                      this->expr.position == node.position;
    },
    VariableExpr, I32Expr, F64Expr, StringExpr, BoolExpr, BinaryExpr,
    UnaryExpr, CallExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, BoolExpr,
    {
        this->value = this->expr.value == node.value &&
                      this->expr.position == node.position;
    },
    VariableExpr, I32Expr, F64Expr, StringExpr, CharExpr, BinaryExpr,
    UnaryExpr, CallExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, UnaryExpr,
    {
        this->value = (*this->expr.expr) == (*node.expr) &&
                      this->expr.op == node.op &&
                      this->expr.position == node.position;
    },
    I32Expr, F64Expr, StringExpr, CharExpr, BoolExpr, BinaryExpr, VariableExpr,
    CallExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, BinaryExpr,
    {
        this->value = (*this->expr.lhs) == (*node.lhs) &&
                      (*this->expr.rhs) == (*node.rhs) &&
                      this->expr.op == node.op &&
                      this->expr.position == node.position;
    },
    I32Expr, F64Expr, StringExpr, CharExpr, BoolExpr, UnaryExpr, VariableExpr,
    CallExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, CallExpr,
    {
        auto are_args_equal = std::equal(
            this->expr.args.begin(), this->expr.args.end(), node.args.begin(),
            [](ExprNodePtr a, ExprNodePtr b) { return *a == *b; });
        this->value = *this->expr.callable == *node.callable &&
                      are_args_equal && this->expr.position == node.position;
    },
    I32Expr, F64Expr, StringExpr, CharExpr, BoolExpr, UnaryExpr, VariableExpr,
    BinaryExpr, LambdaCallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, LambdaCallExpr,
    {
        auto are_args_equal = std::equal(
            this->expr.args.begin(), this->expr.args.end(), node.args.begin(),
            [](ExprNodePtr a, ExprNodePtr b) { return equal_or_null(a, b); });
        this->value = *this->expr.callable == *node.callable &&
                      are_args_equal && this->expr.position == node.position;
    },
    I32Expr, F64Expr, StringExpr, CharExpr, BoolExpr, UnaryExpr, VariableExpr,
    BinaryExpr, CallExpr, IndexExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, IndexExpr,
    {
        this->value = *this->expr.expr == *node.expr &&
                      *this->expr.index_value == *node.index_value &&
                      this->expr.position == node.position;
    },
    I32Expr, F64Expr, StringExpr, CharExpr, BoolExpr, UnaryExpr, VariableExpr,
    BinaryExpr, CallExpr, LambdaCallExpr, CastExpr)

GENERATE_VISITOR(
    ExprVisitor, CastExpr,
    {
        this->value = *this->expr.expr == *node.expr &&
                      *this->expr.type == *node.type &&
                      this->expr.position == node.position;
    },
    I32Expr, F64Expr, StringExpr, CharExpr, BoolExpr, UnaryExpr, VariableExpr,
    BinaryExpr, CallExpr, LambdaCallExpr, IndexExpr)

#define MAKE_EQUATION_VISIT(type)                                             \
    void visit(const type &other) override                                    \
    {                                                                         \
        type##Visitor visitor(other);                                         \
        this->expr.accept(visitor);                                           \
        value = visitor.value;                                                \
    }
#define MAKE_EQUATION_VISITS(...) FOR_EACH(MAKE_EQUATION_VISIT, __VA_ARGS__)

struct ExprEquationVisitor : ExprVisitor
{
    bool value;

    ExprEquationVisitor(const ExprNode &expr) : expr(expr), value(true)
    {
    }
    MAKE_EQUATION_VISITS(VariableExpr, I32Expr, F64Expr, StringExpr, CharExpr,
                         BoolExpr, BinaryExpr, UnaryExpr, CallExpr,
                         LambdaCallExpr, IndexExpr, CastExpr)
  private:
    const ExprNode &expr;
};

bool operator==(const ExprNode &first, const ExprNode &second)
{
    auto visitor = ExprEquationVisitor(first);
    second.accept(visitor);
    return visitor.value;
}

GENERATE_VISITOR(
    StmtVisitor, Block,
    {
        auto are_statements_equal = std::equal(
            this->expr.statements.begin(), this->expr.statements.end(),
            node.statements.begin(),
            [](const StmtPtr &a, const StmtPtr &b) { return *a == *b; });
        this->value =
            are_statements_equal && this->expr.position == node.position;
    },
    IfStmt, WhileStmt, MatchStmt, ReturnStmt, BreakStmt, ContinueStmt,
    FuncDefStmt, AssignStmt, VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, IfStmt,
    {
        this->value = *this->expr.condition_expr == *node.condition_expr &&
                      *this->expr.then_block == *node.then_block &&
                      equal_or_null(this->expr.else_block, other.else_block) &&
                      this->expr.position == node.position;
    },
    Block, WhileStmt, MatchStmt, ReturnStmt, BreakStmt, ContinueStmt,
    FuncDefStmt, AssignStmt, VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, WhileStmt,
    {
        this->value = *this->expr.condition_expr == *node.condition_expr &&
                      *this->expr.block == *node.block &&
                      this->expr.position == node.position;
    },
    Block, IfStmt, MatchStmt, ReturnStmt, BreakStmt, ContinueStmt, FuncDefStmt,
    AssignStmt, VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, MatchStmt,
    {
        auto are_arms_equal =
            std::equal(this->expr.match_arms.begin(),
                       this->expr.match_arms.end(), node.match_arms.begin(),
                       [](const MatchArmPtr &a, const MatchArmPtr &b) {
                           return *a == *b;
                       });
        this->value = *this->expr.matched_expr == *node.matched_expr &&
                      are_arms_equal && this->expr.position == node.position;
    },
    Block, IfStmt, WhileStmt, ReturnStmt, BreakStmt, ContinueStmt, FuncDefStmt,
    AssignStmt, VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, ReturnStmt,
    {
        this->value = equal_or_null(this->expr.expr, node.expr) &&
                      this->expr.position == node.position;
    },
    Block, IfStmt, WhileStmt, MatchStmt, BreakStmt, ContinueStmt, FuncDefStmt,
    AssignStmt, VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, BreakStmt,
    { this->value = this->expr.position == node.position; }, Block, IfStmt,
    WhileStmt, MatchStmt, ReturnStmt, ContinueStmt, FuncDefStmt, AssignStmt,
    VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, BreakStmt,
    { this->value = this->expr.position == node.position; }, Block, IfStmt,
    WhileStmt, MatchStmt, ReturnStmt, ContinueStmt, FuncDefStmt, AssignStmt,
    VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, ContinueStmt,
    { this->value = this->expr.position == node.position; }, Block, IfStmt,
    WhileStmt, MatchStmt, ReturnStmt, BreakStmt, FuncDefStmt, AssignStmt,
    VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, FuncDefStmt,
    {
        auto are_params_equal = std::equal(
            this->expr.params.begin(), this->expr.params.end(),
            node.params.begin(),
            [](const ParamPtr &a, const ParamPtr &b) { return *a == *b; });
        this->value =
            this->expr.name == node.name && are_params_equal &&
            equal_or_null(this->expr.return_type, node.return_type) &&
            *this->expr.block == *node.block &&
            this->expr.is_const == node.is_const &&
            this->expr.position == node.position;
    },
    Block, IfStmt, WhileStmt, MatchStmt, ReturnStmt, BreakStmt, ContinueStmt,
    AssignStmt, VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, AssignStmt,
    {
        this->value = *this->expr.lhs == *node.lhs &&
                      this->expr.type == node.type &&
                      *this->expr.rhs == *node.rhs &&
                      this->expr.position == node.position;
    },
    Block, IfStmt, WhileStmt, MatchStmt, ReturnStmt, BreakStmt, ContinueStmt,
    FuncDefStmt, VarDeclStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, VarDeclStmt,
    {
        this->value =
            this->expr.name == node.name &&
            equal_or_null(this->expr.type, node.type) &&
            equal_or_null(this->expr.initial_value, node.initial_value) &&
            this->expr.is_mut == node.is_mut &&
            this->expr.position == node.position;
    },
    Block, IfStmt, WhileStmt, MatchStmt, ReturnStmt, BreakStmt, ContinueStmt,
    FuncDefStmt, AssignStmt, ExternStmt)

GENERATE_VISITOR(
    StmtVisitor, ExternStmt,
    {
        auto are_params_equal = std::equal(
            this->expr.params.begin(), this->expr.params.end(),
            node.params.begin(),
            [](const ParamPtr &a, const ParamPtr &b) { return *a == *b; });
        this->value =
            this->expr.name == node.name && are_params_equal &&
            equal_or_null(this->expr.return_type, node.return_type) &&
            this->expr.position == node.position;
    },
    Block, IfStmt, WhileStmt, MatchStmt, ReturnStmt, BreakStmt, ContinueStmt,
    FuncDefStmt, AssignStmt, VarDeclStmt)

struct StmtEquationVisitor : StmtVisitor
{
    bool value;

    StmtEquationVisitor(const Statement &expr) : expr(expr), value(true)
    {
    }
    MAKE_EQUATION_VISITS(Block, IfStmt, WhileStmt, MatchStmt, ReturnStmt,
                         BreakStmt, ContinueStmt, FuncDefStmt, AssignStmt,
                         VarDeclStmt, ExternStmt)
  private:
    const Statement &expr;
};

bool operator==(const Statement &first, const Statement &second)
{
    auto visitor = StmtEquationVisitor(first);
    second.accept(visitor);
    return visitor.value;
}