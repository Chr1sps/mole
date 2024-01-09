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
        bool is_signed;
        llvm::Value *last_value;

        // std::unordered_map<> variable_map;
        // std::unordered_map<std::wstring,
        // std::unordered_map<
        std::vector<std::unordered_map<std::wstring, llvm::Value *>> variables;
        std::unordered_map<std::wstring, llvm::Value *> globals;

        llvm::Value *find_variable(const std::wstring &name) const;

        llvm::Value *create_unsigned_binop(llvm::Value *lhs, llvm::Value *rhs,
                                           const BinOpEnum &op);
        llvm::Value *create_signed_binop(llvm::Value *lhs, llvm::Value *rhs,
                                         const BinOpEnum &op);
        llvm::Value *create_double_binop(llvm::Value *lhs, llvm::Value *rhs,
                                         const BinOpEnum &op);
        llvm::Value *Visitor::create_string_binop(llvm::Value *lhs,
                                                  llvm::Value *rhs,
                                                  const BinOpEnum &op);
        void visit(const BinaryExpr &node);
        void visit(const UnaryExpr &node);
        void visit(const IndexExpr &node);
        void visit(const CastExpr &node);

      public:
        Visitor() noexcept;
        Visitor(const Visitor &) = delete;
        Visitor(Visitor &&) = default;

        void visit(const ExternStmt &node);

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