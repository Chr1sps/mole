#include "ir_generator.hpp"

IRGenerator::Visitor::Visitor() noexcept
    : context(std::make_unique<llvm::LLVMContext>())
{
    this->module = std::make_unique<llvm::Module>("mole", *this->context);
    this->builder = std::make_unique<llvm::IRBuilder<>>(*this->context);
}

IRGenerator::IRGenerator() noexcept : visitor()
{
}

void IRGenerator::Visitor::visit(const BinaryExpr &node)
{
}

void IRGenerator::Visitor::visit(const Expression &node)
{
    std::visit(
        overloaded{
            // [this](const VariableExpr &node) {
            //     // this->visit(node);
            // },
            [this](const U32Expr &node) {
                // TODO: store this somewhere
                llvm::ConstantInt::get(this->builder->getInt32Ty(),
                                       node.value);
            },
            [this](const F64Expr &node) {
                // TODO: store this somewhere
                llvm::ConstantFP::get(this->builder->getDoubleTy(),
                                      node.value);
            },
            [this](const StringExpr &node) {
                const char *data =
                    reinterpret_cast<const char *>(node.value.c_str());
                std::size_t size = (node.value.length() + 1) * sizeof(wchar_t);
                // TODO: store this somewhere
                this->builder->CreateBitCast(
                    this->builder->CreateGlobalStringPtr(
                        llvm::StringRef(data, size)),
                    llvm::Type::getInt32PtrTy(*this->context));
            },
            [this](const CharExpr &node) {
                // TODO: store this somewhere
                llvm::ConstantInt::get(this->builder->getInt32Ty(),
                                       node.value);
            },
            [this](const BoolExpr &node) {
                // TODO: store this somewhere
                llvm::ConstantInt::getBool(this->builder->getInt1Ty(),
                                           node.value);
            },
            [this](const BinaryExpr &node) { this->visit(node); },
            //    [this](const UnaryExpr &node) { this->visit(node); },
            //    [this](const CallExpr &node) { this->visit(node); },
            //    [this](const LambdaCallExpr &node) { this->visit(node); },
            //    [this](const IndexExpr &node) { this->visit(node); },
            //    [this](const CastExpr &node) { this->visit(node); },
            [](const auto &) {}},

        node);
}

void IRGenerator::Visitor::visit(const Statement &node)
{
    std::visit(
        overloaded{
            //    [this](const Block &node) { this->visit(node); },
            //    [this](const IfStmt &node) { this->visit(node); },
            //    [this](const WhileStmt &node) { this->visit(node); },
            //    [this](const MatchStmt &node) { this->visit(node); },
            //    [this](const ReturnStmt &node) { this->visit(node); },
            //    [this](const BreakStmt &node) { this->visit(node); },
            //    [this](const ContinueStmt &node) { this->visit(node); },
            //    [this](const FuncDefStmt &node) { this->visit(node); },
            //    [this](const AssignStmt &node) { this->visit(node); },
            //    [this](const ExprStmt &node) { this->visit(node); },
            //    [this](const VarDeclStmt &node) { this->visit(node); },
            //    [this](const ExternStmt &node) { this->visit(node); },
            [](const auto &) {}},
        node);
}

void IRGenerator::Visitor::visit(const MatchArm &node)
{
}