#include "compiled_program.hpp"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include <iostream>
#include <ranges>

CompiledProgram::Visitor::Visitor(const Program &program)
    : context(std::make_unique<llvm::LLVMContext>())
{

    std::string logs;

    auto target_triple = llvm::sys::getDefaultTargetTriple();
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, logs);
    if (!target)
        throw CompilationException(logs.c_str());
    llvm::TargetOptions target_options;
    std::unique_ptr<llvm::TargetMachine> target_machine(
        target->createTargetMachine(target_triple, "generic", "",
                                    target_options, llvm::Reloc::PIC_));
    auto data_layout = target_machine->createDataLayout();

    this->module = std::make_unique<llvm::Module>("mole", *this->context);
    this->module->setTargetTriple(target_triple);
    this->module->setDataLayout(data_layout);

    this->builder = std::make_unique<llvm::IRBuilder<>>(*this->context);
    this->visit(program);

    auto out = llvm::raw_string_ostream(logs);

    if (llvm::verifyModule(*this->module, &out))
    {
        std::string ir;
        auto ir_stream = llvm::raw_string_ostream(ir);
        ir_stream << *this->module;

        throw CompilationException(ir + "\n" + logs);
    }
}

CompiledProgram::CompiledProgram(const Program &program) : visitor(program)
{
}

void CompiledProgram::Visitor::enter_scope()
{
    this->variables.push_back({});
}

void CompiledProgram::Visitor::leave_scope()
{
    this->variables.pop_back();
}

llvm::FunctionType *CompiledProgram::Visitor::get_fn_type(const FuncDef &node)
{
    std::vector<llvm::Type *> param_types;
    for (const auto &param_type :
         node.params | std::views::transform(
                           [](const ParamPtr &param) { return param->type; }))
    {
        param_types.push_back(this->get_var_type(param_type));
    }
    auto return_type = (node.return_type)
                           ? (this->get_var_type(*node.return_type))
                           : (llvm::Type::getVoidTy(*this->context));
    return llvm::FunctionType::get(return_type, param_types, false);
}

llvm::FunctionType *CompiledProgram::Visitor::get_fn_type(
    const ExternDef &node)
{
    std::vector<llvm::Type *> param_types;
    for (const auto &param_type :
         node.params | std::views::transform(
                           [](const ParamPtr &param) { return param->type; }))
    {
        param_types.push_back(this->get_var_type(param_type));
    }
    auto return_type = (node.return_type)
                           ? (this->get_var_type(*node.return_type))
                           : (llvm::Type::getVoidTy(*this->context));
    return llvm::FunctionType::get(return_type, param_types, false);
}

llvm::Type *CompiledProgram::Visitor::get_var_type(const Type &type)
{
    if (type.ref_spec == RefSpecifier::NON_REF)
    {
        switch (type.type)
        {
        case TypeEnum::U32:
        case TypeEnum::I32:
        case TypeEnum::CHAR:
            return llvm::Type::getInt32Ty(*this->context);

        case TypeEnum::BOOL:
            return llvm::Type::getInt1Ty(*this->context);

        case TypeEnum::F64:
            return llvm::Type::getDoubleTy(*this->context);

        default:
            [[unlikely]] throw std::runtime_error("unreachable");
            return nullptr;
        }
    }
    else
    {
        switch (type.type)
        {
        case TypeEnum::U32:
        case TypeEnum::I32:
        case TypeEnum::CHAR:
        case TypeEnum::STR:
            return llvm::Type::getInt32PtrTy(*this->context);
            break;

        case TypeEnum::BOOL:
            return llvm::Type::getInt1PtrTy(*this->context);
            break;

        case TypeEnum::F64:
            return llvm::Type::getDoublePtrTy(*this->context);
        }
    }
}

CompiledProgram::Visitor::Value CompiledProgram::Visitor::find_variable(
    const std::wstring &name) const
{
    for (const auto &scope : variables)
    {
        auto found = scope.find(name);
        if (found != scope.cend())
        {
            return found->second;
        }
    }
    return this->globals.find(name)->second;
}

void CompiledProgram::Visitor::create_unsigned_binop(llvm::Value *lhs,
                                                     llvm::Value *rhs,
                                                     const BinOpEnum &op)
{
    llvm::Value *new_value;
    llvm::Type *new_type = this->builder->getInt32Ty();
    switch (op)
    {
    case BinOpEnum::ADD:
        new_value = this->builder->CreateAdd(lhs, rhs, "add_u32");
        break;
    case BinOpEnum::SUB:
        new_value = this->builder->CreateSub(lhs, rhs, "sub_u32");
        break;
    case BinOpEnum::MUL:
        new_value = this->builder->CreateMul(lhs, rhs, "mul_u32");
        break;
    case BinOpEnum::DIV:
        new_value = this->builder->CreateUDiv(lhs, rhs, "div_u32");
        break;
    case BinOpEnum::MOD:
        new_value = this->builder->CreateURem(lhs, rhs, "mod_u32");
        break;
    case BinOpEnum::EXP:
        new_value = this->builder->CreateCall(
            llvm::Intrinsic::getDeclaration(this->module.get(),
                                            llvm::Intrinsic::powi),
            {lhs, rhs});
        break;
    case BinOpEnum::EQ:
        new_value = this->builder->CreateICmpEQ(lhs, rhs, "eq_u32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::NEQ:
        new_value = this->builder->CreateICmpNE(lhs, rhs, "neq_u32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::GT:
        new_value = this->builder->CreateICmpUGT(lhs, rhs, "gt_u32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::GE:
        new_value = this->builder->CreateICmpUGE(lhs, rhs, "ge_u32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::LT:
        new_value = this->builder->CreateICmpULT(lhs, rhs, "lt_u32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::LE:
        new_value = this->builder->CreateICmpULE(lhs, rhs, "le_u32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::AND:
        new_value = this->builder->CreateAnd(lhs, rhs, "and_bool");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::BIT_AND:
        new_value = this->builder->CreateAnd(lhs, rhs, "bit_and_u32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::OR:
        new_value = this->builder->CreateOr(lhs, rhs, "or_bool");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::BIT_OR:
        new_value = this->builder->CreateOr(lhs, rhs, "bit_or_u32");
        break;
    case BinOpEnum::BIT_XOR:
        new_value = this->builder->CreateXor(lhs, rhs, "bit_xor_u32");
        break;
    case BinOpEnum::SHL:
        new_value = this->builder->CreateShl(lhs, rhs, "shl_u32");
        break;
    case BinOpEnum::SHR:
        new_value = this->builder->CreateAShr(lhs, rhs, "shr_u32");
        break;

    default:
        [[unlikely]] throw std::runtime_error("unreachable");
    }
    this->last_value = Value(new_value, new_type);
}

void CompiledProgram::Visitor::create_signed_binop(llvm::Value *lhs,
                                                   llvm::Value *rhs,
                                                   const BinOpEnum &op)
{
    llvm::Value *new_value;
    llvm::Type *new_type = this->builder->getInt32Ty();
    switch (op)
    {
    case BinOpEnum::ADD:
        new_value = this->builder->CreateAdd(lhs, rhs, "add_i32");
        break;
    case BinOpEnum::SUB:
        new_value = this->builder->CreateSub(lhs, rhs, "sub_i32");
        break;
    case BinOpEnum::MUL:
        new_value = this->builder->CreateMul(lhs, rhs, "mul_i32");
        break;
    case BinOpEnum::DIV:
        new_value = this->builder->CreateSDiv(lhs, rhs, "div_i32");
        break;
    case BinOpEnum::MOD:
        new_value = this->builder->CreateSRem(lhs, rhs, "mod_i32");
        break;
    case BinOpEnum::EXP:
        new_value = this->builder->CreateCall(
            llvm::Intrinsic::getDeclaration(this->module.get(),
                                            llvm::Intrinsic::powi),
            {lhs, rhs});
        break;
    case BinOpEnum::EQ:
        new_value = this->builder->CreateICmpEQ(lhs, rhs, "eq_i32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::NEQ:
        new_value = this->builder->CreateICmpNE(lhs, rhs, "neq_i32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::GT:
        new_value = this->builder->CreateICmpSGT(lhs, rhs, "gt_i32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::GE:
        new_value = this->builder->CreateICmpSGE(lhs, rhs, "ge_i32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::LT:
        new_value = this->builder->CreateICmpSLT(lhs, rhs, "lt_i32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::LE:
        new_value = this->builder->CreateICmpSLE(lhs, rhs, "le_i32");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::BIT_AND:
        new_value = this->builder->CreateAnd(lhs, rhs, "bit_and_i32");
        break;
    case BinOpEnum::BIT_OR:
        new_value = this->builder->CreateOr(lhs, rhs, "bit_or_i32");
        break;
    case BinOpEnum::BIT_XOR:
        new_value = this->builder->CreateXor(lhs, rhs, "bit_xor_i32");
        break;
    case BinOpEnum::SHL:
        new_value = this->builder->CreateShl(lhs, rhs, "shl_i32");
        break;
    case BinOpEnum::SHR:
        new_value = this->builder->CreateAShr(lhs, rhs, "shr_i32");
        break;

    default:
        [[unlikely]] throw std::runtime_error("unreachable");
    }
    this->last_value = Value(new_value, new_type);
}

void CompiledProgram::Visitor::create_double_binop(llvm::Value *lhs,
                                                   llvm::Value *rhs,
                                                   const BinOpEnum &op)
{
    llvm::Value *new_value;
    llvm::Type *new_type = this->builder->getDoubleTy();
    switch (op)
    {
    case BinOpEnum::ADD:
        new_value = this->builder->CreateFAdd(lhs, rhs, "add_f64");
        break;
    case BinOpEnum::SUB:
        new_value = this->builder->CreateFSub(lhs, rhs, "sub_f64");
        break;
    case BinOpEnum::MUL:
        new_value = this->builder->CreateFMul(lhs, rhs, "mul_f64");
        break;
    case BinOpEnum::DIV:
        new_value = this->builder->CreateFDiv(lhs, rhs, "div_f64");
        break;
    case BinOpEnum::MOD:
        new_value = this->builder->CreateFRem(lhs, rhs, "mod_f64");
        break;
    case BinOpEnum::EXP:
        new_value = this->builder->CreateCall(
            llvm::Intrinsic::getDeclaration(this->module.get(),
                                            llvm::Intrinsic::pow),
            {lhs, rhs});
        break;
    case BinOpEnum::EQ:
        new_value = this->builder->CreateFCmpOEQ(lhs, rhs, "eq_f64");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::NEQ:
        new_value = this->builder->CreateFCmpONE(lhs, rhs, "neq_f64");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::GT:
        new_value = this->builder->CreateFCmpOGT(lhs, rhs, "gt_f64");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::GE:
        new_value = this->builder->CreateFCmpOGE(lhs, rhs, "ge_f64");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::LT:
        new_value = this->builder->CreateFCmpOLT(lhs, rhs, "lt_f64");
        new_type = this->builder->getInt1Ty();
        break;
    case BinOpEnum::LE:
        new_value = this->builder->CreateFCmpOLE(lhs, rhs, "le_f64");
        new_type = this->builder->getInt1Ty();
        break;

    default:
        [[unlikely]] throw std::runtime_error("unreachable");
    }
    this->last_value = Value(new_value, new_type);
}

llvm::Value *CompiledProgram::Visitor::get_dereferenced_value(
    const Value &value)
{
    if (value.type->isPointerTy())
    {
        auto type = llvm::dyn_cast<llvm::PointerType>(value.type)
                        ->getArrayElementType();
        return this->builder->CreateLoad(type, value.address);
    }
    else
        return value.value;
}

void CompiledProgram::Visitor::visit(const BinaryExpr &node)
{
    this->visit(*node.lhs);
    auto lhs = this->last_value;
    auto lhs_value = this->get_dereferenced_value(lhs);

    this->visit(*node.rhs);
    auto rhs = this->last_value;
    auto rhs_value = this->get_dereferenced_value(rhs);

    if (lhs.type->isIntegerTy())
    {
        if (this->is_signed)
        {
            this->create_signed_binop(lhs_value, rhs_value, node.op);
        }
        else
            this->create_unsigned_binop(lhs_value, rhs_value, node.op);
    }
    else
        this->create_double_binop(lhs_value, rhs_value, node.op);
}

void CompiledProgram::Visitor::visit(const UnaryExpr &node)
{
    this->visit(*node.expr);
    auto expr = this->last_value;
    llvm::Value *new_value, *new_ptr = nullptr;
    llvm::Type *new_type;
    switch (node.op)
    {
    case UnaryOpEnum::MINUS:
        if (expr.type->isIntegerTy())
        {
            if (!this->is_signed)
            {
                new_value = this->builder->CreateZExt(
                    expr.value, llvm::Type::getInt32Ty(*this->context),
                    "u32_to_i32");
                new_value = this->builder->CreateNeg(new_value);
            }
            else
                new_value = this->builder->CreateNeg(expr.value);
            new_type = this->builder->getInt32Ty();
        }
        else
        {
            new_value = this->builder->CreateFNeg(expr.value);
            new_type = expr.type;
        }
        break;

    case UnaryOpEnum::NEG:
    case UnaryOpEnum::BIT_NEG:
        new_value = this->builder->CreateNot(expr.value);
        new_type = expr.type;
        break;
    case UnaryOpEnum::REF:
    case UnaryOpEnum::MUT_REF:
        new_value = expr.address;
        new_type = expr.type->getPointerTo();
        break;
    case UnaryOpEnum::DEREF:
        new_value = this->builder->CreateLoad(expr.type, expr.value);
        // new_type =
        // llvm::cast<llvm::PointerType>(expr.type)->getContainedType();
        new_type = new_value->getType();
        new_ptr = expr.value;
        break;
    }
    this->last_value = Value(new_value, new_type, new_ptr);
}

void CompiledProgram::Visitor::visit_call(const CallExpr &node)
{
    auto callable = this->functions.at(node.callable);
    std::vector<llvm::Value *> args;
    for (const auto &arg : node.args)
    {
        this->visit(*arg);
        args.push_back(this->last_value.value);
    }
    auto type = callable.ptr->getReturnType();
    auto value = this->builder->CreateCall(callable.ptr, args);
    this->last_value = Value(value, type);
}

void CompiledProgram::Visitor::visit(const IndexExpr &node)
{
    this->visit(*node.expr);
    auto expr = this->last_value.value;
    this->visit(*node.index_value);
    auto index = this->last_value.value;
    auto address =
        this->builder->CreateGEP(this->builder->getInt32Ty(), expr, index);

    auto value = this->builder->CreateSExtOrTrunc(
        this->builder->CreateLoad(this->builder->getInt32Ty(), address),
        this->builder->getInt32Ty());
    auto type = this->builder->getInt32Ty();
    this->last_value = Value(value, type);
}

void CompiledProgram::Visitor::visit(const CastExpr &node)
{
    this->visit(*node.expr);
    auto value = this->last_value.value;
    auto from_type = this->last_value.type;
    from_type->isIntegerTy();
    auto type = node.type;
    llvm::Value *new_value;
    llvm::Type *new_type;
    switch (type.type)
    {
    case TypeEnum::U32:
        if (from_type->isIntegerTy(1))

            new_value = this->builder->CreateZExt(
                value, llvm::Type::getInt32Ty(*this->context), "bool_to_u32");

        else if (from_type->isIntegerTy(32) && this->is_signed)
            new_value = this->builder->CreateSExt(
                value, llvm::Type::getInt32Ty(*this->context), "i32_to_u32");

        else
            new_value = this->builder->CreateFPToUI(
                value, llvm::Type::getInt32Ty(*this->context), "f64_to_u32");
        new_type = this->builder->getInt32Ty();
        break;
    case TypeEnum::CHAR:
        new_value = value;
        new_type = this->builder->getInt32Ty();
        break;

    case TypeEnum::I32:
        if (from_type->isIntegerTy(1))
            new_value = this->builder->CreateZExt(
                value, llvm::Type::getInt32Ty(*this->context), "bool_to_i32");

        else if (from_type->isIntegerTy(32) && !this->is_signed)
            new_value = this->builder->CreateZExt(
                value, llvm::Type::getInt32Ty(*this->context), "u32_to_i32");

        else
            new_value = this->builder->CreateFPToSI(
                value, llvm::Type::getInt32Ty(*this->context), "f64_to_i32");
        new_type = this->builder->getInt32Ty();
        break;

    case TypeEnum::F64:
        if (from_type->isIntegerTy(1))
            new_value = this->builder->CreateUIToFP(
                value, llvm::Type::getDoubleTy(*this->context), "bool_to_f64");

        else
        {
            if (this->is_signed)
                new_value = this->builder->CreateSIToFP(
                    value, llvm::Type::getDoubleTy(*this->context),
                    "i32_to_f64");
            else
                new_value = this->builder->CreateUIToFP(
                    value, llvm::Type::getDoubleTy(*this->context),
                    "u32_to_f64");
        }
        new_type = this->builder->getDoubleTy();
        break;

    case TypeEnum::BOOL:
        new_value = value;
        new_type = this->builder->getInt1Ty();
        break;

    default:
        throw std::runtime_error("unreachable");
        break;
    }
    this->last_value = Value(new_value, new_type);
}

void CompiledProgram::Visitor::visit(const Expression &node)
{
    std::visit(
        overloaded{[this](const VariableExpr &node) {
                       auto variable = this->find_variable(node.name);
                       auto new_value = this->builder->CreateLoad(
                           variable.type, variable.address);
                       this->last_value =
                           Value(new_value, variable.type, variable.address);
                   },
                   [this](const U32Expr &node) {
                       auto value = llvm::ConstantInt::get(
                           this->builder->getInt32Ty(), node.value);
                       auto type = this->builder->getInt32Ty();
                       this->last_value = Value(value, type);
                       this->is_signed = false;
                   },
                   [this](const F64Expr &node) {
                       auto value = llvm::ConstantFP::get(
                           this->builder->getDoubleTy(), node.value);
                       auto type = this->builder->getDoubleTy();
                       this->last_value = Value(value, type);
                   },
                   [this](const StringExpr &node) {
                       const char *data =
                           reinterpret_cast<const char *>(node.value.c_str());
                       std::size_t size =
                           (node.value.length() + 1) * sizeof(wchar_t);
                       auto value = this->builder->CreateBitCast(
                           this->builder->CreateGlobalStringPtr(
                               llvm::StringRef(data, size)),
                           llvm::Type::getInt32PtrTy(*this->context));
                       auto type = llvm::Type::getInt32PtrTy(*this->context);
                       this->last_value = Value(value, type);
                   },
                   [this](const CharExpr &node) {
                       auto value = llvm::ConstantInt::get(
                           this->builder->getInt32Ty(), node.value);
                       auto type = this->builder->getInt32Ty();
                       this->last_value = Value(value, type);
                   },
                   [this](const BoolExpr &node) {
                       auto value = llvm::ConstantInt::getBool(
                           this->builder->getInt1Ty(), node.value);
                       auto type = this->builder->getInt1Ty();
                       this->last_value = Value(value, type);
                   },
                   [this](const BinaryExpr &node) { this->visit(node); },
                   [this](const UnaryExpr &node) { this->visit(node); },
                   [this](const CallExpr &node) { this->visit_call(node); },
                   [this](const IndexExpr &node) { this->visit(node); },
                   [this](const CastExpr &node) { this->visit(node); }},

        node);
}

void CompiledProgram::Visitor::visit_block(const Block &node)
{
    auto is_return_covered = false;
    this->enter_scope();
    for (const auto &stmt : node.statements)
    {
        this->visit(*stmt);
        is_return_covered |= this->is_return_covered;
    }
    this->leave_scope();
    this->is_return_covered = is_return_covered;
}

void CompiledProgram::Visitor::visit(const IfStmt &node)
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

    this->builder->CreateBr(condition);
    this->builder->SetInsertPoint(condition);
    this->visit(*node.condition_expr);
    auto condition_value = this->last_value.value;
    this->builder->CreateCondBr(condition_value, then_entry,
                                (node.else_block) ? (else_entry) : (exit));

    this->builder->SetInsertPoint(then_entry);
    this->visit(*node.then_block);
    auto is_then_covered = this->is_return_covered;
    this->builder->CreateBr(exit);

    if (node.else_block)
    {
        this->builder->SetInsertPoint(else_entry);
        this->visit(*node.else_block);
        this->builder->CreateBr(exit);
        this->is_return_covered &= is_then_covered;
    }
    else
    {
        this->is_return_covered = false;
    }
    this->builder->SetInsertPoint(exit);
}

void CompiledProgram::Visitor::visit(const WhileStmt &node)
{
    auto condition = llvm::BasicBlock::Create(
        *this->context, "while_condition_expr", this->current_function);
    auto entry = llvm::BasicBlock::Create(*this->context, "while_body",
                                          this->current_function);
    auto exit = llvm::BasicBlock::Create(*this->context, "while_exit",
                                         this->current_function);

    auto previous_entry = std::exchange(this->loop_entry, entry);
    auto previous_exit = std::exchange(this->loop_exit, exit);

    this->builder->CreateBr(condition);
    this->builder->SetInsertPoint(condition);
    this->visit(*node.condition_expr);
    auto condition_value = this->last_value.value;
    this->builder->CreateCondBr(condition_value, entry, exit);
    this->builder->SetInsertPoint(entry);
    this->visit(*node.statement);
    this->builder->CreateBr(condition);
    this->builder->SetInsertPoint(exit);

    this->loop_entry = previous_entry;
    this->loop_exit = previous_exit;
    this->is_return_covered = false;
}

void CompiledProgram::Visitor::visit(const ReturnStmt &node)
{
    if (node.expr)
    {
        this->visit(*node.expr);
        this->builder->CreateRet(this->last_value.value);
    }
    else
        this->builder->CreateRetVoid();
    this->is_return_covered = true;
}

void CompiledProgram::Visitor::visit(const MatchStmt &node)
{
    auto exit = llvm::BasicBlock::Create(*this->context, "match_exit",
                                         this->current_function);
    auto condition = llvm::BasicBlock::Create(
        *this->context, "match_condition", this->current_function);

    this->visit(*node.matched_expr);
    auto matched_value = this->last_value.value;

    auto previous_condition = std::exchange(this->match_condition, condition);
    auto previous_exit = std::exchange(this->match_exit, exit);
    auto previous_matched = std::exchange(this->matched_value, matched_value);
    auto previous_is_exhaustive = std::exchange(this->is_exhaustive, false);

    this->builder->CreateBr(condition);
    this->builder->SetInsertPoint(condition);

    auto is_return_covered = true;
    for (const auto &arm : node.match_arms)
    {
        this->visit(*arm);
        is_return_covered &= this->is_return_covered;
        if (this->is_exhaustive)
            break;
    }
    this->is_return_covered = is_return_covered;

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

void CompiledProgram::Visitor::visit(const AssignStmt &node)
{
    this->visit(*node.lhs);
    auto ptr = this->last_value.address;
    this->visit(*node.rhs);
    auto value = this->last_value.value;
    this->builder->CreateStore(value, ptr);
    this->is_return_covered = false;
}

void CompiledProgram::Visitor::declare_global(const VarDeclStmt &node)
{
    this->visit(*node.initial_value);
    auto value = this->last_value;
    auto type = (node.type) ? (this->get_var_type(*node.type)) : (value.type);

    auto ptr = new llvm::GlobalVariable(
        *this->module, type, !node.is_mut, llvm::GlobalValue::ExternalLinkage,
        llvm::cast<llvm::Constant>(value.value));
    this->globals.insert({node.name, Value{ptr, type}});
}

void CompiledProgram::Visitor::visit(const VarDeclStmt &node)
{
    this->visit(*node.initial_value);
    auto value = this->last_value.value;
    auto type = (node.type) ? (this->get_var_type(*node.type))
                            : (this->last_value.type);

    auto ptr = this->builder->CreateAlloca(type);
    this->builder->CreateStore(value, ptr);
    this->variables.back().insert({node.name, Value{value, type, ptr}});
    this->is_return_covered = false;
}

void CompiledProgram::Visitor::declare_func(const FuncDef &node)
{
    auto type = this->get_fn_type(node);
    auto name = std::string(node.name.cbegin(), node.name.cend());
    auto func = llvm::Function::Create(type, llvm::Function::ExternalLinkage,
                                       name, *this->module);
    func->setCallingConv(llvm::CallingConv::C);
    func->arg_begin();

    this->functions.insert({node.name, Function{func, type}});
}

void CompiledProgram::Visitor::visit(const FuncDef &node)
{
    auto func = this->functions.at(node.name);

    auto entry =
        llvm::BasicBlock::Create(*this->context, "fn_entry", func.ptr);
    this->builder->SetInsertPoint(entry);
    this->enter_scope();
    for (const auto &[param, arg] :
         std::views::zip(node.params, func.ptr->args()))
    {
        auto param_ptr = this->builder->CreateAlloca(arg.getType());
        this->variables.back().insert(
            {param->name, Value{param_ptr, arg.getType(), param_ptr}});
        this->builder->CreateStore(&arg, param_ptr);
    }
    this->current_function = func.ptr;
    this->visit_block(*node.block);
    if (!this->is_return_covered && !node.return_type)
        this->builder->CreateRetVoid();

    this->leave_scope();
}

void CompiledProgram::Visitor::visit(const ExternDef &node)
{
    auto type = this->get_fn_type(node);
    auto name = std::string(node.name.cbegin(), node.name.cend());
    auto func = llvm::Function::Create(type, llvm::Function::ExternalLinkage,
                                       name, *this->module);
    func->setCallingConv(llvm::CallingConv::C);
    func->setName(name);
    this->functions.insert({node.name, Function{func, type}});
}

void CompiledProgram::Visitor::visit(const Statement &node)
{
    std::visit(
        overloaded{[this](const Block &node) { this->visit_block(node); },
                   [this](const WhileStmt &node) { this->visit(node); },
                   [this](const ReturnStmt &node) { this->visit(node); },
                   [this](const BreakStmt &node) {
                       this->builder->CreateBr(this->loop_exit);
                       this->is_return_covered = false;
                   },
                   [this](const ContinueStmt &node) {
                       this->builder->CreateBr(this->loop_entry);
                       this->is_return_covered = false;
                   },
                   [this](const IfStmt &node) { this->visit(node); },
                   [this](const MatchStmt &node) { this->visit(node); },
                   [this](const AssignStmt &node) { this->visit(node); },
                   [this](const ExprStmt &node) { this->visit(*node.expr); },
                   [this](const VarDeclStmt &node) { this->visit(node); }},
        node);
}

llvm::Value *CompiledProgram::Visitor::create_literal_condition_value(
    const Expression &expr)
{
    this->visit(expr);
    auto value = this->last_value.value;
    if (value->getType()->isIntegerTy())
        // this also handles bools
        return this->builder->CreateICmpEQ(this->matched_value, value);
    else
        return this->builder->CreateFCmpOEQ(this->matched_value, value);
}

void CompiledProgram::Visitor::visit(const LiteralArm &node)
{
    auto stmt = llvm::BasicBlock::Create(*this->context, "match_stmt",
                                         this->current_function);

    llvm::BasicBlock *new_condition;
    for (const auto &literal : node.literals)
    {
        new_condition = llvm::BasicBlock::Create(
            *this->context, "match_condition", this->current_function);
        auto condition_value = this->create_literal_condition_value(*literal);
        this->builder->CreateCondBr(condition_value, stmt, new_condition);
        this->builder->SetInsertPoint(new_condition);
    }
    this->builder->SetInsertPoint(stmt);
    this->visit(*node.block);
    if (!this->is_return_covered)
        this->builder->CreateBr(this->match_exit);
    this->builder->SetInsertPoint(new_condition);
    this->match_condition = new_condition;
}

void CompiledProgram::Visitor::visit(const GuardArm &node)
{
    auto stmt = llvm::BasicBlock::Create(*this->context, "match_stmt",
                                         this->current_function);
    auto new_condition = llvm::BasicBlock::Create(
        *this->context, "match_condition", this->current_function);
    this->visit(*node.condition_expr);
    auto condition_value = this->last_value.value;
    this->builder->CreateCondBr(condition_value, stmt, new_condition);
    this->builder->SetInsertPoint(stmt);
    this->visit(*node.block);
    if (!this->is_return_covered)
        this->builder->CreateBr(this->match_exit);
    this->builder->SetInsertPoint(new_condition);
    this->match_condition = new_condition;
}

void CompiledProgram::Visitor::visit(const ElseArm &node)
{
    auto stmt = llvm::BasicBlock::Create(*this->context, "match_stmt",
                                         this->current_function);
    this->builder->CreateBr(stmt);
    this->builder->SetInsertPoint(stmt);
    this->visit(*node.block);
    if (!this->is_return_covered)
        this->builder->CreateBr(this->match_exit);
    this->is_exhaustive = true;
}

void CompiledProgram::Visitor::visit(const MatchArm &node)
{
    std::visit(
        overloaded{[this](const LiteralArm &node) { this->visit(node); },
                   [this](const GuardArm &node) { this->visit(node); },
                   [this](const ElseArm &node) { this->visit(node); }},
        node);
}

void CompiledProgram::Visitor::visit(const Program &node)
{
    for (const auto &func : node.functions)
        this->declare_func(*func);
    for (const auto &ext : node.externs)
        this->visit(*ext);
    for (const auto &var : node.globals)
        this->declare_global(*var);
    for (const auto &func : node.functions)
        this->visit(*func);
}

void CompiledProgram::output_bytecode(llvm::raw_fd_ostream &output)
{
    llvm::WriteBitcodeToFile(*this->visitor.module, output);
}

void CompiledProgram::output_ir(llvm::raw_fd_ostream &output)
{
    output << *this->visitor.module;
}