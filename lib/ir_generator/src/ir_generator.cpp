#include "ir_generator.hpp"

IRGenerator::Visitor::Visitor() noexcept
    : context(std::make_unique<llvm::LLVMContext>())
{
    this->module = std::make_unique<llvm::Module>("mole", *this->context);
    this->builder = std::make_unique<llvm::IRBuilder<>>(*this->context);
}

void IRGenerator::Visitor::enter_scope()
{
    this->variables.push_back({});
}

void IRGenerator::Visitor::leave_scope()
{
    this->variables.pop_back();
}

llvm::Value *IRGenerator::Visitor::find_variable(
    const std::wstring &name) const
{
    for (const auto &scope : variables)
    {
        auto found = scope.find(name);
        if (found != scope.cend())
            return found->second;
    }

    return this->globals.find(name)->second;
}

void IRGenerator::Visitor::create_unsigned_binop(llvm::Value *lhs,
                                                 llvm::Value *rhs,
                                                 const BinOpEnum &op)
{
    switch (op)
    {
    case BinOpEnum::ADD:
        this->last_value = this->builder->CreateAdd(lhs, rhs, "add_u32");
        break;
    case BinOpEnum::SUB:
        this->last_value = this->builder->CreateSub(lhs, rhs, "sub_u32");
        break;
    case BinOpEnum::MUL:
        this->last_value = this->builder->CreateMul(lhs, rhs, "mul_u32");
        break;
    case BinOpEnum::DIV:
        this->last_value = this->builder->CreateUDiv(lhs, rhs, "div_u32");
        break;
    case BinOpEnum::MOD:
        this->last_value = this->builder->CreateURem(lhs, rhs, "mod_u32");
        break;
    case BinOpEnum::EXP:
        this->last_value = this->builder->CreateCall(
            llvm::Intrinsic::getDeclaration(this->module.get(),
                                            llvm::Intrinsic::powi),
            {lhs, rhs});
        break;
    case BinOpEnum::EQ:
        this->last_value = this->builder->CreateICmpEQ(lhs, rhs, "eq_u32");
        break;
    case BinOpEnum::NEQ:
        this->last_value = this->builder->CreateICmpNE(lhs, rhs, "neq_u32");
        break;
    case BinOpEnum::GT:
        this->last_value = this->builder->CreateICmpUGT(lhs, rhs, "gt_u32");
        break;
    case BinOpEnum::GE:
        this->last_value = this->builder->CreateICmpUGE(lhs, rhs, "ge_u32");
        break;
    case BinOpEnum::LT:
        this->last_value = this->builder->CreateICmpULT(lhs, rhs, "lt_u32");
        break;
    case BinOpEnum::LE:
        this->last_value = this->builder->CreateICmpULE(lhs, rhs, "le_u32");
        break;
    case BinOpEnum::AND:
        this->last_value = this->builder->CreateAnd(lhs, rhs, "and_bool");
        break;
    case BinOpEnum::BIT_AND:
        this->last_value = this->builder->CreateAnd(lhs, rhs, "bit_and_u32");
        break;
    case BinOpEnum::OR:
        this->last_value = this->builder->CreateOr(lhs, rhs, "or_bool");
        break;
    case BinOpEnum::BIT_OR:
        this->last_value = this->builder->CreateOr(lhs, rhs, "bit_or_u32");
        break;
    case BinOpEnum::BIT_XOR:
        this->last_value = this->builder->CreateXor(lhs, rhs, "bit_xor_u32");
        break;
    case BinOpEnum::SHL:
        this->last_value = this->builder->CreateShl(lhs, rhs, "shl_u32");
        break;
    case BinOpEnum::SHR:
        this->last_value = this->builder->CreateAShr(lhs, rhs, "shr_u32");
        break;

    default:
        [[unlikely]] throw std::runtime_error("unreachable");
    }
}

void IRGenerator::Visitor::create_signed_binop(llvm::Value *lhs,
                                               llvm::Value *rhs,
                                               const BinOpEnum &op)
{
    switch (op)
    {
    case BinOpEnum::ADD:
        this->last_value = this->builder->CreateAdd(lhs, rhs, "add_i32");
        break;
    case BinOpEnum::SUB:
        this->last_value = this->builder->CreateSub(lhs, rhs, "sub_i32");
        break;
    case BinOpEnum::MUL:
        this->last_value = this->builder->CreateMul(lhs, rhs, "mul_i32");
        break;
    case BinOpEnum::DIV:
        this->last_value = this->builder->CreateSDiv(lhs, rhs, "div_i32");
        break;
    case BinOpEnum::MOD:
        this->last_value = this->builder->CreateSRem(lhs, rhs, "mod_i32");
        break;
    case BinOpEnum::EXP:
        this->last_value = this->builder->CreateCall(
            llvm::Intrinsic::getDeclaration(this->module.get(),
                                            llvm::Intrinsic::powi),
            {lhs, rhs});
        break;
    case BinOpEnum::EQ:
        this->last_value = this->builder->CreateICmpEQ(lhs, rhs, "eq_i32");
        break;
    case BinOpEnum::NEQ:
        this->last_value = this->builder->CreateICmpNE(lhs, rhs, "neq_i32");
        break;
    case BinOpEnum::GT:
        this->last_value = this->builder->CreateICmpSGT(lhs, rhs, "gt_i32");
        break;
    case BinOpEnum::GE:
        this->last_value = this->builder->CreateICmpSGE(lhs, rhs, "ge_i32");
        break;
    case BinOpEnum::LT:
        this->last_value = this->builder->CreateICmpSLT(lhs, rhs, "lt_i32");
        break;
    case BinOpEnum::LE:
        this->last_value = this->builder->CreateICmpSLE(lhs, rhs, "le_i32");
        break;
    case BinOpEnum::BIT_AND:
        this->last_value = this->builder->CreateAnd(lhs, rhs, "bit_and_i32");
        break;
    case BinOpEnum::BIT_OR:
        this->last_value = this->builder->CreateOr(lhs, rhs, "bit_or_i32");
        break;
    case BinOpEnum::BIT_XOR:
        this->last_value = this->builder->CreateXor(lhs, rhs, "bit_xor_i32");
        break;
    case BinOpEnum::SHL:
        this->last_value = this->builder->CreateShl(lhs, rhs, "shl_i32");
        break;
    case BinOpEnum::SHR:
        this->last_value = this->builder->CreateAShr(lhs, rhs, "shr_i32");
        break;

    default:
        [[unlikely]] throw std::runtime_error("unreachable");
    }
}

void IRGenerator::Visitor::create_double_binop(llvm::Value *lhs,
                                               llvm::Value *rhs,
                                               const BinOpEnum &op)
{
    switch (op)
    {
    case BinOpEnum::ADD:
        this->last_value = this->builder->CreateFAdd(lhs, rhs, "add_f64");
        break;
    case BinOpEnum::SUB:
        this->last_value = this->builder->CreateFSub(lhs, rhs, "sub_f64");
        break;
    case BinOpEnum::MUL:
        this->last_value = this->builder->CreateFMul(lhs, rhs, "mul_f64");
        break;
    case BinOpEnum::DIV:
        this->last_value = this->builder->CreateFDiv(lhs, rhs, "div_f64");
        break;
    case BinOpEnum::MOD:
        this->last_value = this->builder->CreateFRem(lhs, rhs, "mod_f64");
        break;
    case BinOpEnum::EXP:
        this->last_value = this->builder->CreateCall(
            llvm::Intrinsic::getDeclaration(this->module.get(),
                                            llvm::Intrinsic::pow),
            {lhs, rhs});
        break;
    case BinOpEnum::EQ:
        this->last_value = this->builder->CreateFCmpOEQ(lhs, rhs, "eq_f64");
        break;
    case BinOpEnum::NEQ:
        this->last_value = this->builder->CreateFCmpONE(lhs, rhs, "neq_f64");
        break;
    case BinOpEnum::GT:
        this->last_value = this->builder->CreateFCmpOGT(lhs, rhs, "gt_f64");
        break;
    case BinOpEnum::GE:
        this->last_value = this->builder->CreateFCmpOGE(lhs, rhs, "ge_f64");
        break;
    case BinOpEnum::LT:
        this->last_value = this->builder->CreateFCmpOLT(lhs, rhs, "lt_f64");
        break;
    case BinOpEnum::LE:
        this->last_value = this->builder->CreateFCmpOLE(lhs, rhs, "le_f64");
        break;

    default:
        [[unlikely]] throw std::runtime_error("unreachable");
    }
}

void IRGenerator::Visitor::create_string_binop(llvm::Value *lhs,
                                               llvm::Value *rhs,
                                               const BinOpEnum &op)
{
    // TODO
    switch (op)
    {
    case BinOpEnum::ADD:
        this->last_value = this->builder->CreateFAdd(lhs, rhs, "add_f64");
        break;
    case BinOpEnum::EQ:
        this->last_value = this->builder->CreateFCmpOEQ(lhs, rhs, "eq_f64");
        break;
    case BinOpEnum::NEQ:
        this->last_value = this->builder->CreateFCmpONE(lhs, rhs, "neq_f64");
        break;

    default:
        [[unlikely]] throw std::runtime_error("unreachable");
    }
}

void IRGenerator::Visitor::visit(const BinaryExpr &node)
{
    this->visit(*node.lhs);
    auto lhs = this->last_value;

    this->visit(*node.rhs);
    auto rhs = this->last_value;

    if (lhs->getType()->isPointerTy())
        this->create_string_binop(lhs, rhs, node.op);
    else if (lhs->getType()->isIntegerTy())
    {
        if (this->is_signed)
            this->create_signed_binop(lhs, rhs, node.op);
        else
            this->create_unsigned_binop(lhs, rhs, node.op);
    }
    else if (lhs->getType()->isDoubleTy())
        this->create_double_binop(lhs, rhs, node.op);
}

void IRGenerator::Visitor::visit(const UnaryExpr &node)
{
}

void IRGenerator::Visitor::visit(const IndexExpr &node)
{
    this->visit(*node.expr);
    auto expr = this->last_value;
    this->visit(*node.index_value);
    auto index = this->last_value;
    auto address =
        this->builder->CreateGEP(this->builder->getInt32Ty(), expr, index);
    this->last_value = this->builder->CreateSExtOrTrunc(
        this->builder->CreateLoad(this->builder->getInt32Ty(), address),
        this->builder->getInt32Ty());
}

void IRGenerator::Visitor::visit(const CastExpr &node)
{
    this->visit(*node.expr);
    auto value = this->last_value;
    auto from_type = value->getType();
    from_type->isIntegerTy();
    auto type = std::get<SimpleType>(*node.type);
    switch (type.type)
    {
    case TypeEnum::U32:
        if (from_type->isIntegerTy(1))

            this->last_value = this->builder->CreateZExt(
                this->last_value, llvm::Type::getInt32Ty(*this->context),
                "bool_to_u32");

        else if (from_type->isIntegerTy(32) && this->is_signed)
            this->last_value = this->builder->CreateSExt(
                this->last_value, llvm::Type::getInt32Ty(*this->context),
                "i32_to_u32");

        else if (from_type->isDoubleTy())
            this->last_value = this->builder->CreateFPToUI(
                this->last_value, llvm::Type::getInt32Ty(*this->context),
                "f64_to_u32");
        break;

    case TypeEnum::I32:
        if (from_type->isIntegerTy(1))
            this->last_value = this->builder->CreateZExt(
                this->last_value, llvm::Type::getInt32Ty(*this->context),
                "bool_to_i32");

        else if (from_type->isIntegerTy(32) && !this->is_signed)
            this->last_value = this->builder->CreateZExt(
                this->last_value, llvm::Type::getInt32Ty(*this->context),
                "u32_to_i32");

        else if (from_type->isDoubleTy())
            this->last_value = this->builder->CreateFPToSI(
                this->last_value, llvm::Type::getInt32Ty(*this->context),
                "f64_to_i32");
        break;

    case TypeEnum::F64:
        if (from_type->isIntegerTy(1))
            this->last_value = this->builder->CreateUIToFP(
                this->last_value, llvm::Type::getDoubleTy(*this->context),
                "bool_to_f64");

        else if (from_type->isIntegerTy(32))
        {
            if (this->is_signed)
                this->last_value = this->builder->CreateSIToFP(
                    this->last_value, llvm::Type::getDoubleTy(*this->context),
                    "i32_to_f64");
            else
                this->last_value = this->builder->CreateUIToFP(
                    this->last_value, llvm::Type::getDoubleTy(*this->context),
                    "u32_to_f64");
        }
        break;

    default:
        // Casting to bool is only allowed for bool values and casting to char
        // from u32 doesn't do anything, so we can default it.
        break;
    }
}

void IRGenerator::Visitor::visit(const Expression &node)
{
    std::visit(
        overloaded{
            [this](const VariableExpr &node) {
                this->last_value = this->find_variable(node.name);
            },
            [this](const U32Expr &node) {
                this->last_value = llvm::ConstantInt::get(
                    this->builder->getInt32Ty(), node.value);
                this->is_signed = false;
            },
            [this](const F64Expr &node) {
                this->last_value = llvm::ConstantFP::get(
                    this->builder->getDoubleTy(), node.value);
            },
            [this](const StringExpr &node) {
                const char *data =
                    reinterpret_cast<const char *>(node.value.c_str());
                std::size_t size = (node.value.length() + 1) * sizeof(wchar_t);
                this->last_value = this->builder->CreateBitCast(
                    this->builder->CreateGlobalStringPtr(
                        llvm::StringRef(data, size)),
                    llvm::Type::getInt32PtrTy(*this->context));
            },
            [this](const CharExpr &node) {
                this->last_value = llvm::ConstantInt::get(
                    this->builder->getInt32Ty(), node.value);
            },
            [this](const BoolExpr &node) {
                this->last_value = llvm::ConstantInt::getBool(
                    this->builder->getInt1Ty(), node.value);
            },
            [this](const BinaryExpr &node) { this->visit(node); },
            // [this](const UnaryExpr &node) { this->visit(node); },
            //    [this](const CallExpr &node) { this->visit(node); },
            //    [this](const LambdaCallExpr &node) { this->visit(node); },
            [this](const IndexExpr &node) { this->visit(node); },
            [this](const CastExpr &node) { this->visit(node); },
            [](const auto &) {}},

        node);
}

void IRGenerator::Visitor::visit_block(const Block &node)
{
    this->enter_scope();
    for (const auto &stmt : node.statements)
    {
        this->visit(*stmt);
    }
    this->leave_scope();
}

void IRGenerator::Visitor::visit(const IfStmt &node)
{
    auto condition = llvm::BasicBlock::Create(
        *this->context, "if_condition_expr", this->current_function);
    auto then_entry = llvm::BasicBlock::Create(*this->context, "if_then_entry",
                                               this->current_function);
    llvm::BasicBlock *else_entry;
    if (node.else_block)
    {
        else_entry = llvm::BasicBlock::Create(*this->context, "if_else_entry",
                                              this->current_function);
    }
    auto exit = llvm::BasicBlock::Create(*this->context, "if_exit",
                                         this->current_function);

    this->visit(*node.condition_expr);
    auto condition_value = this->last_value;

    this->builder->CreateBr(condition);
    this->builder->SetInsertPoint(condition);
    this->builder->CreateCondBr(condition_value, then_entry,
                                (node.else_block) ? (else_entry) : (exit));

    this->builder->SetInsertPoint(then_entry);
    this->visit(*node.then_block);
    this->builder->CreateBr(exit);

    if (node.else_block)
    {
        this->builder->SetInsertPoint(else_entry);
        this->visit(*node.else_block);
        this->builder->CreateBr(exit);
    }
    this->builder->SetInsertPoint(exit);
}

void IRGenerator::Visitor::visit(const WhileStmt &node)
{
    auto condition = llvm::BasicBlock::Create(
        *this->context, "while_condition_expr", this->current_function);
    auto entry = llvm::BasicBlock::Create(*this->context, "while_body",
                                          this->current_function);
    auto exit = llvm::BasicBlock::Create(*this->context, "while_exit",
                                         this->current_function);

    auto previous_entry = std::exchange(this->loop_entry, entry);
    auto previous_exit = std::exchange(this->loop_exit, exit);

    this->visit(*node.condition_expr);
    auto condition_value = this->last_value;

    this->builder->CreateBr(condition);
    this->builder->SetInsertPoint(condition);
    this->builder->CreateCondBr(condition_value, entry, exit);
    this->builder->SetInsertPoint(entry);
    this->visit(*node.statement);
    this->builder->CreateBr(condition);
    this->builder->SetInsertPoint(exit);

    this->loop_entry = previous_entry;
    this->loop_exit = previous_exit;
}

void IRGenerator::Visitor::visit(const ReturnStmt &node)
{
    if (node.expr)
    {
        this->visit(*node.expr);
        this->builder->CreateRet(this->last_value);
    }
    else
        this->builder->CreateRetVoid();
}

void IRGenerator::Visitor::visit(const MatchStmt &node)
{
    auto exit = llvm::BasicBlock::Create(*this->context, "match_exit",
                                         this->current_function);
    auto condition = llvm::BasicBlock::Create(
        *this->context, "match_condition", this->current_function);

    this->visit(*node.matched_expr);
    auto matched_value = this->last_value;

    auto previous_condition = std::exchange(this->match_condition, condition);
    auto previous_exit = std::exchange(this->match_exit, exit);
    auto previous_matched = std::exchange(this->matched_value, matched_value);
    auto previous_is_exhaustive = std::exchange(this->is_exhaustive, false);

    this->builder->CreateBr(condition);
    this->builder->SetInsertPoint(condition);

    for (const auto &arm : node.match_arms)
    {
        this->visit(*arm);
        if (this->is_exhaustive)
            break;
    }

    if (!this->is_exhaustive)
    {
        this->builder->CreateBr(exit);
    }
    this->builder->SetInsertPoint(exit);

    this->match_condition = previous_condition;
    this->match_exit = previous_exit;
    this->matched_value = previous_matched;
    this->is_exhaustive = previous_is_exhaustive;
}

void IRGenerator::Visitor::visit(const ExternStmt &node)
{
}

void IRGenerator::Visitor::visit(const Statement &node)
{
    std::visit(
        overloaded{
            [this](const Block &node) { this->visit_block(node); },
            [this](const WhileStmt &node) { this->visit(node); },
            [this](const ReturnStmt &node) { this->visit(node); },
            [this](const BreakStmt &node) {
                this->builder->CreateBr(this->loop_exit);
            },
            [this](const ContinueStmt &node) {
                this->builder->CreateBr(this->loop_entry);
            },
            [this](const IfStmt &node) { this->visit(node); },
            [this](const MatchStmt &node) { this->visit(node); },
            //    [this](const AssignStmt &node) { this->visit(node); },
            [this](const ExprStmt &node) { this->visit(*node.expr); },
            //    [this](const VarDeclStmt &node) { this->visit(node); },
            //    [this](const FuncDefStmt &node) { this->visit(node); },
            [this](const ExternStmt &node) { this->visit(node); },
            [](const auto &) {}},
        node);
}

llvm::Value *IRGenerator::Visitor::create_literal_condition_value(
    const Expression &expr)
{
    this->visit(expr);
    auto value = this->last_value;
    if (value->getType()->isPointerTy())
        // TODO: handle strings
        return nullptr;
    else if (value->getType()->isIntegerTy())
        // this also handles bools
        return this->builder->CreateICmpEQ(this->matched_value, value);
    else
        return this->builder->CreateFCmpOEQ(this->matched_value, value);
}

void IRGenerator::Visitor::visit(const LiteralArm &node)
{
    auto stmt = llvm::BasicBlock::Create(*this->context, "match_stmt",
                                         this->current_function);

    this->builder->SetInsertPoint(stmt);
    this->visit(*node.block);
    this->builder->CreateBr(this->match_exit);
    llvm::BasicBlock *new_condition;
    for (const auto &literal : node.literals)
    {
        new_condition = llvm::BasicBlock::Create(
            *this->context, "match_condition", this->current_function);
        auto condition_value = this->create_literal_condition_value(*literal);
        this->builder->CreateCondBr(condition_value, stmt, new_condition);
        this->builder->SetInsertPoint(new_condition);
    }
    this->match_condition = new_condition;
}

void IRGenerator::Visitor::visit(const GuardArm &node)
{
    auto stmt = llvm::BasicBlock::Create(*this->context, "match_stmt",
                                         this->current_function);
    auto new_condition = llvm::BasicBlock::Create(
        *this->context, "match_condition", this->current_function);
    this->visit(*node.condition_expr);
    auto condition_value = this->last_value;
    this->builder->CreateCondBr(condition_value, stmt, new_condition);
    this->builder->SetInsertPoint(stmt);
    this->visit(*node.block);
    this->builder->CreateBr(this->match_exit);
    this->builder->SetInsertPoint(new_condition);
    this->match_condition = new_condition;
}

void IRGenerator::Visitor::visit(const ElseArm &node)
{
    auto stmt = llvm::BasicBlock::Create(*this->context, "match_stmt",
                                         this->current_function);
    this->builder->CreateBr(stmt);
    this->builder->SetInsertPoint(stmt);
    this->visit(*node.block);
    this->builder->CreateBr(this->match_exit);
    this->is_exhaustive = true;
}

void IRGenerator::Visitor::visit(const MatchArm &node)
{
    std::visit(
        overloaded{[this](const LiteralArm &node) { this->visit(node); },
                   [this](const GuardArm &node) { this->visit(node); },
                   [this](const ElseArm &node) { this->visit(node); }},
        node);
}