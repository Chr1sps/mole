#ifndef __IR_GENERATOR_HPP__
#define __IR_GENERATOR_HPP__
#include "visitor.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Target/TargetMachine.h"
#include <filesystem>

class CompiledProgram
{

    class Visitor : public ExprVisitor,
                    public StmtVisitor,
                    public ProgramVisitor,
                    public MatchArmVisitor
    {
        struct Function
        {
            llvm::Function *ptr;
            llvm::Type *type;
        };

        struct Value
        {
            llvm::Value *value;
            llvm::Type *type;
            llvm::Value *address;

            Value() = default;

            Value(llvm::Value *ptr, llvm::Type *type,
                  llvm::Value *address = nullptr)
                : value(ptr), type(type), address(address)
            {
            }
        };

        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unique_ptr<llvm::TargetMachine> target_machine;
        bool is_signed, is_exhaustive, is_return_covered;
        llvm::Value *matched_value;
        Value last_value;
        llvm::Function *current_function;
        llvm::BasicBlock *loop_entry, *loop_exit, *match_condition,
            *match_exit;

        std::vector<std::unordered_map<std::wstring, Value>> variables;
        std::unordered_map<std::wstring, Function> functions;
        std::unordered_map<std::wstring, Value> globals;

        void enter_scope();
        void leave_scope();

        llvm::FunctionType *get_fn_type(const FuncDef &node);
        llvm::FunctionType *get_fn_type(const ExternDef &node);
        llvm::Type *get_var_type(const Type &type);
        Value find_variable(const std::wstring &name) const;
        Function find_function(const std::wstring &name) const;

        void create_unsigned_binop(llvm::Value *lhs, llvm::Value *rhs,
                                   const BinOpEnum &op);
        void create_signed_binop(llvm::Value *lhs, llvm::Value *rhs,
                                 const BinOpEnum &op);
        void create_double_binop(llvm::Value *lhs, llvm::Value *rhs,
                                 const BinOpEnum &op);
        void create_string_binop(llvm::Value *lhs, llvm::Value *rhs,
                                 const BinOpEnum &op);
        llvm::Value *get_dereferenced_value(const Value &value);
        void visit(const BinaryExpr &node);
        void visit(const UnaryExpr &node);
        void visit_call(const CallExpr &node);
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

        void declare_global(const VarDeclStmt &node);
        void visit(const VarDeclStmt &node);
        void declare_func(const FuncDef &node);
        void visit(const FuncDef &node);
        void visit(const ExternDef &node);

        llvm::Value *create_literal_condition_value(const Expression &expr);
        void visit(const LiteralArm &node);
        void visit(const GuardArm &node);
        void visit(const ElseArm &node);

        void visit(const Expression &node) override;
        void visit(const Statement &node) override;
        void visit(const MatchArm &node) override;

        void visit(const Program &node) override;

        void optimize();
        void output_object_file(llvm::raw_fd_ostream &output);
    } visitor;

  public:
    CompiledProgram(const Program &);
    CompiledProgram(const CompiledProgram &) = delete;
    CompiledProgram(CompiledProgram &&) = default;

    void output_ir(llvm::raw_fd_ostream &output);
    void output_bytecode(llvm::raw_fd_ostream &output);
    void output_object_file(llvm::raw_fd_ostream &output);
    void optimize();
    // int execute();
};

class CompilationException : std::runtime_error
{
    const std::string what_str;

  public:
    CompilationException(const std::string &what_str)
        : std::runtime_error(what_str), what_str(what_str)
    {
    }

    const char *what() const noexcept override
    {
        return this->what_str.c_str();
    }
};
#endif