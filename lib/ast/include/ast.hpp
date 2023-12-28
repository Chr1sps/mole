#ifndef __AST_HPP__
#define __AST_HPP__
#include "position.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

enum class TypeEnum
{
    BOOL,

    I32,

    U32,

    F64,

    CHAR,
    STR,
};

enum class RefSpecifier
{
    NON_REF,
    REF,
    MUT_REF
};

struct SimpleType;
struct FunctionType;
using Type = std::variant<SimpleType, FunctionType>;
using TypePtr = std::unique_ptr<Type>;

struct SimpleType
{
    TypeEnum type;
    RefSpecifier ref_spec;

    SimpleType(const TypeEnum &type, const RefSpecifier &ref_spec) noexcept;
    SimpleType(const SimpleType &) noexcept = default;
    SimpleType(SimpleType &&) noexcept = default;
};

struct FunctionType
{
    std::vector<TypePtr> arg_types;
    TypePtr return_type;
    bool is_const;

    FunctionType(std::vector<TypePtr> arg_types, TypePtr return_type,
                 const bool &is_const) noexcept;

    FunctionType(const FunctionType &other) noexcept;

    FunctionType(FunctionType &&) noexcept = default;
};

bool operator==(const Type &, const Type &) noexcept;

struct AstNode
{

    Position position;

    virtual ~AstNode()
    {
    }

  protected:
    AstNode(const Position &position) noexcept : position(position)
    {
    }
};

using AstNodePtr = std::unique_ptr<AstNode>;

struct VariableExpr;
struct BinaryExpr;
struct UnaryExpr;
struct CallExpr;
struct LambdaCallExpr;
struct IndexExpr;
struct CastExpr;
struct U32Expr;
struct F64Expr;
struct BoolExpr;
struct StringExpr;
struct CharExpr;

using ExprNode = std::variant<VariableExpr, BinaryExpr, UnaryExpr, CallExpr,
                              LambdaCallExpr, IndexExpr, CastExpr, U32Expr,
                              F64Expr, BoolExpr, StringExpr, CharExpr>;
using ExprNodePtr = std::unique_ptr<ExprNode>;

struct VariableExpr : public AstNode
{
    std::wstring name;

    VariableExpr(const std::wstring &name, const Position &position) noexcept;
};

enum class BinOpEnum
{
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    EXP,

    EQ,
    NEQ,
    GT,
    GE,
    LT,
    LE,

    AND,
    OR,

    BIT_AND,
    BIT_OR,
    BIT_XOR,

    SHL,
    SHR,
};

enum class UnaryOpEnum
{
    INC,
    DEC,

    NEG,
    BIT_NEG,
    MINUS,
    REF
};

struct BinaryExpr : public AstNode
{
    ExprNodePtr lhs, rhs;
    BinOpEnum op;

    BinaryExpr(ExprNodePtr lhs, ExprNodePtr rhs, const BinOpEnum &op,
               const Position &position) noexcept;
};

struct UnaryExpr : public AstNode
{
    ExprNodePtr expr;
    UnaryOpEnum op;

    UnaryExpr(ExprNodePtr expr, const UnaryOpEnum &op,
              const Position &position) noexcept;
};

struct CallExpr : public AstNode
{
    ExprNodePtr callable;
    std::vector<ExprNodePtr> args;

    CallExpr(ExprNodePtr callable, std::vector<ExprNodePtr> args,
             const Position &position) noexcept;
};

struct LambdaCallExpr : public AstNode
{
    ExprNodePtr callable;
    std::vector<ExprNodePtr> args;

    LambdaCallExpr(ExprNodePtr callable, std::vector<ExprNodePtr> args,
                   const Position &position) noexcept;
};

struct IndexExpr : public AstNode
{
    ExprNodePtr expr, index_value;

    IndexExpr(ExprNodePtr expr, ExprNodePtr index_value,
              const Position &position) noexcept;
};

struct CastExpr : public AstNode
{
    ExprNodePtr expr;
    TypePtr type;

    CastExpr(ExprNodePtr expr, TypePtr type,
             const Position &position) noexcept;
};

struct U32Expr : public AstNode
{
    static const std::shared_ptr<SimpleType> type;
    unsigned long long value;

    U32Expr(const unsigned long long &value,
            const Position &position) noexcept;
};

struct F64Expr : public AstNode
{
    static const std::shared_ptr<SimpleType> type;
    double value;

    F64Expr(const double &value, const Position &position) noexcept;
};

struct StringExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    std::wstring value;

    StringExpr(const std::wstring &value, const Position &position) noexcept;
};

struct CharExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    wchar_t value;

    CharExpr(const wchar_t &value, const Position &position) noexcept;
};

struct BoolExpr : public AstNode
{
    // static const std::shared_ptr<SimpleType> type;
    bool value;

    BoolExpr(const bool &value, const Position &position) noexcept;
};

bool operator==(const ExprNode &, const ExprNode &) noexcept;

Position get_expr_position(const ExprNode &expr);
void set_expr_position(ExprNode &expr, const Position &position) noexcept;

struct Block;
struct IfStmt;
struct WhileStmt;
struct MatchStmt;
struct BreakStmt;
struct ContinueStmt;
struct ReturnStmt;
struct FuncDefStmt;
struct AssignStmt;
struct ExprStmt;
struct VarDeclStmt;
struct ExternStmt;
using Statement = std::variant<Block, ReturnStmt, ContinueStmt, BreakStmt,
                               VarDeclStmt, AssignStmt, ExprStmt, WhileStmt,
                               IfStmt, MatchStmt, FuncDefStmt, ExternStmt>;
using StmtPtr = std::unique_ptr<Statement>;
using BlockPtr = std::unique_ptr<Block>;

struct Block : public AstNode
{
    std::vector<StmtPtr> statements;

    Block(std::vector<StmtPtr> statements, const Position &position) noexcept;
};

bool operator==(const Block &, const Block &) noexcept;

struct ReturnStmt : public AstNode
{
    ExprNodePtr expr;

    ReturnStmt(const Position &position) noexcept;
    ReturnStmt(ExprNodePtr expr, const Position &position) noexcept;
};

struct ContinueStmt : public AstNode
{
    ContinueStmt(const Position &position) noexcept;
};

struct BreakStmt : public AstNode
{
    BreakStmt(const Position &position) noexcept;
};

enum class AssignType
{
    NORMAL,

    PLUS,
    MINUS,
    MUL,
    DIV,
    MOD,
    EXP,

    BIT_NEG,
    BIT_AND,
    BIT_OR,
    BIT_XOR,

    SHL,
    SHR,
};

struct AssignStmt : public AstNode
{
    ExprNodePtr lhs, rhs;
    AssignType type;

    AssignStmt(ExprNodePtr lhs, const AssignType &type, ExprNodePtr rhs,
               const Position &position) noexcept;
};

struct ExprStmt : public AstNode
{
    ExprNodePtr expr;

    ExprStmt(ExprNodePtr expr, const Position &position) noexcept;
};

struct WhileStmt : public AstNode
{
    ExprNodePtr condition_expr;
    StmtPtr statement;

    WhileStmt(ExprNodePtr condition_expr, StmtPtr statement,
              const Position &position) noexcept;
};

struct IfStmt : public AstNode
{
    ExprNodePtr condition_expr;
    StmtPtr then_block;
    StmtPtr else_block;

    IfStmt(ExprNodePtr condition_expr, StmtPtr then_block, StmtPtr else_block,
           const Position &position) noexcept;
};

struct LiteralArm;
struct GuardArm;
struct ElseArm;

using MatchArm = std::variant<LiteralArm, GuardArm, ElseArm>;
using MatchArmPtr = std::unique_ptr<MatchArm>;

struct MatchStmt : public AstNode
{
    ExprNodePtr matched_expr;
    std::vector<MatchArmPtr> match_arms;

    MatchStmt(ExprNodePtr matched_expr, std::vector<MatchArmPtr> match_arms,
              const Position &position) noexcept;
};

struct VarDeclStmt : public AstNode
{
    std::wstring name;
    TypePtr type;
    ExprNodePtr initial_value;
    bool is_mut;

    VarDeclStmt(const std::wstring &name, TypePtr type, ExprNodePtr value,
                const bool &is_mut, const Position &position) noexcept;
};

bool operator==(const VarDeclStmt &, const VarDeclStmt &) noexcept;

struct Parameter;

using ParamPtr = std::unique_ptr<Parameter>;

struct FuncDefStmt : public AstNode
{
    std::wstring name;
    std::vector<ParamPtr> params;
    TypePtr return_type;
    BlockPtr block;
    bool is_const;

    FuncDefStmt(const std::wstring &name, std::vector<ParamPtr> params,
                TypePtr return_type, BlockPtr block, const bool &is_const,
                const Position &position) noexcept;

    std::unique_ptr<FunctionType> get_type() const noexcept;
};

bool operator==(const FuncDefStmt &, const FuncDefStmt &) noexcept;

struct ExternStmt : public AstNode
{
    std::wstring name;
    std::vector<ParamPtr> params;
    TypePtr return_type;

    ExternStmt(const std::wstring &name, std::vector<ParamPtr> params,
               TypePtr return_type, const Position &position) noexcept;

    std::unique_ptr<FunctionType> get_type() const noexcept;
};

bool operator==(const ExternStmt &, const ExternStmt &) noexcept;

bool operator==(const Statement &, const Statement &) noexcept;

struct MatchArmBase : public AstNode
{
    StmtPtr block;

    MatchArmBase(StmtPtr block, const Position &position) noexcept;
};

struct ElseArm : public MatchArmBase
{
    ElseArm(StmtPtr block, const Position &position) noexcept;
};

struct GuardArm : public MatchArmBase
{
    ExprNodePtr condition_expr;

    GuardArm(ExprNodePtr condition_expr, StmtPtr block,
             const Position &position) noexcept;
};

struct LiteralArm : public MatchArmBase

{
    std::vector<ExprNodePtr> literals;

    LiteralArm(std::vector<ExprNodePtr> literals, StmtPtr block,
               const Position &position) noexcept;
};

bool operator==(const MatchArm &, const MatchArm &) noexcept;

struct Program : public AstNode
{
    std::vector<std::unique_ptr<VarDeclStmt>> globals;
    std::vector<std::unique_ptr<FuncDefStmt>> functions;
    std::vector<std::unique_ptr<ExternStmt>> externs;

    Program(std::vector<std::unique_ptr<VarDeclStmt>> globals,
            std::vector<std::unique_ptr<FuncDefStmt>> functions,
            std::vector<std::unique_ptr<ExternStmt>> externs) noexcept;
};

using ProgramPtr = std::unique_ptr<Program>;

bool operator==(const Program &, const Program &) noexcept;

struct Parameter : public AstNode
{
    std::wstring name;
    TypePtr type;

    Parameter(const std::wstring &name, TypePtr type,
              const Position &position) noexcept;
};

bool operator==(const Parameter &, const Parameter &) noexcept;

#endif