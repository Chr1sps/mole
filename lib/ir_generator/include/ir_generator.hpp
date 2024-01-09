#ifndef __IR_GENERATOR_HPP__
#define __IR_GENERATOR_HPP__
#include "visitor.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <filesystem>

class IRGenerator
{

    class Visitor : public ExprVisitor,
                    public StmtVisitor,
                    public MatchArmVisitor
    {
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::Module> module;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        bool is_signed, is_exhaustive;
        llvm::Value *last_value, *matched_value;
        llvm::Function *current_function;
        llvm::BasicBlock *loop_entry, *loop_exit, *match_condition,
            *match_exit;

        // std::unordered_map<> variable_map;
        // std::unordered_map<std::wstring,
        // std::unordered_map<
        std::vector<std::unordered_map<std::wstring, llvm::Value *>> variables;
        std::unordered_map<std::wstring, llvm::Value *> globals;

        void enter_scope();
        void leave_scope();

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
        void visit(const IndexExpr &node);
        void visit(const CastExpr &node);

      public:
        Visitor() noexcept;
        Visitor(const Visitor &) = delete;
        Visitor(Visitor &&) = default;

        void visit_block(const Block &node);
        void visit(const IfStmt &node);
        void visit(const WhileStmt &node);
        void visit(const ReturnStmt &node);
        void visit(const MatchStmt &node);
        void visit(const ExternStmt &node);

        llvm::Value *create_literal_condition_value(const Expression &expr);
        void visit(const LiteralArm &node);
        void visit(const GuardArm &node);
        void visit(const ElseArm &node);

        void visit(const Expression &node) override;
        void visit(const Statement &node) override;
        void visit(const MatchArm &node) override;
    } visitor;

  public:
    IRGenerator() noexcept = default;
    IRGenerator(const IRGenerator &) = delete;
    IRGenerator(IRGenerator &&) = default;

    void save_ir(const std::filesystem::path &output_path);
    void save_bytecode(const std::filesystem::path &output_path);
};
#endif