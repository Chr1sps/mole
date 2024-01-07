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

        // std::unordered_map<> variable_map;

        void visit(const BinaryExpr &node);

      public:
        Visitor() noexcept;
        Visitor(const Visitor &) = delete;
        Visitor(Visitor &&) = default;

        void visit(const Expression &node) override;
        void visit(const Statement &node) override;
        void visit(const MatchArm &node) override;
    } visitor;

  public:
    IRGenerator() noexcept;
    IRGenerator(const IRGenerator &) = delete;
    IRGenerator(IRGenerator &&) = default;

    void save_ir(const std::filesystem::path &output_path);
    void save_bytecode(const std::filesystem::path &output_path);
};
#endif