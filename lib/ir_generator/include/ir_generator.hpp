#ifndef __IR_GENERATOR_HPP__
#define __IR_GENERATOR_HPP__
#include "visitor.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <filesystem>

class CompiledProgram
{

    class Visitor : public ExprVisitor,
                    public StmtVisitor,
                    public ProgramVisitor,
                    public MatchArmVisitor
    {
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        bool is_signed, is_exhaustive;
        llvm::Value *last_value, *matched_value;
        llvm::Function *current_function;
        llvm::BasicBlock *loop_entry, *loop_exit, *match_condition,
            *match_exit;
        llvm::Type *last_type;

        std::vector<std::unordered_map<std::wstring, llvm::Value *>> variables;
        std::unordered_map<std::wstring, llvm::Function *> functions;
        std::unordered_map<std::wstring, llvm::Value *> globals;

        void enter_scope();
        void leave_scope();

        llvm::FunctionType *get_fn_type(const FunctionType &type);
        llvm::Type *get_var_type(const Type &type);
        llvm::Value *find_variable(const std::wstring &name) const;

        void create_unsigned_binop(llvm::Value *lhs, llvm::Value *rhs,
                                   const BinOpEnum &op);
        void create_signed_binop(llvm::Value *lhs, llvm::Value *rhs,
                                 const BinOpEnum &op);
        void create_double_binop(llvm::Value *lhs, llvm::Value *rhs,
                                 const BinOpEnum &op);
        void create_string_binop(llvm::Value *lhs, llvm::Value *rhs,
                                 const BinOpEnum &op);
        void visit(const BinaryExpr &node);
        void visit(const UnaryExpr &node);
        void visit(const CallExpr &node);
        void visit(const IndexExpr &node);
        void visit(const CastExpr &node);

      public:
        std::unique_ptr<llvm::Module> module;
        Visitor(const Program &);
        Visitor(const Visitor &) = delete;
        Visitor(Visitor &&) = default;

        void visit_block(const Block &node);
        void visit(const IfStmt &node);
        void visit(const WhileStmt &node);
        void visit(const ReturnStmt &node);
        void visit(const MatchStmt &node);
        void visit(const AssignStmt &node);
        void visit(const VarDeclStmt &node);
        void visit(const ExternDef &node);

        void declare_func(const FuncDef &node);
        void visit(const FuncDef &node);

        llvm::Value *create_literal_condition_value(const Expression &expr);
        void visit(const LiteralArm &node);
        void visit(const GuardArm &node);
        void visit(const ElseArm &node);

        void visit(const Expression &node) override;
        void visit(const Statement &node) override;
        void visit(const MatchArm &node) override;

        void create_entrypoint(llvm::Function *main_func);

        void visit(const Program &node) override;
    } visitor;

  public:
    CompiledProgram(const Program &);
    CompiledProgram(const CompiledProgram &) = delete;
    CompiledProgram(CompiledProgram &&) = default;

    void output_ir(llvm::raw_ostream &output);
    void output_bytecode(llvm::raw_ostream &output);
    // int execute();
};

class CompilationException : std::runtime_error
{
  public:
    CompilationException() : std::runtime_error("IR Generator error.")
    {
    }
};
#endif