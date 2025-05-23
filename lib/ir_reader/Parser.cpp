#include "mini-llvm/ir_reader/Parser.h"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <typeinfo>
#include <unordered_set>
#include <utility>
#include <vector>

#include "mini-llvm/common/Linkage.h"
#include "mini-llvm/ir/Argument.h"
#include "mini-llvm/ir/Attribute.h"
#include "mini-llvm/ir/BasicBlock.h"
#include "mini-llvm/ir/Constant.h"
#include "mini-llvm/ir/Constant/ArrayConstant.h"
#include "mini-llvm/ir/Constant/DoubleConstant.h"
#include "mini-llvm/ir/Constant/FloatConstant.h"
#include "mini-llvm/ir/Constant/I16Constant.h"
#include "mini-llvm/ir/Constant/I1Constant.h"
#include "mini-llvm/ir/Constant/I32Constant.h"
#include "mini-llvm/ir/Constant/I64Constant.h"
#include "mini-llvm/ir/Constant/I8Constant.h"
#include "mini-llvm/ir/Constant/NullPtrConstant.h"
#include "mini-llvm/ir/Constant/PoisonValue.h"
#include "mini-llvm/ir/Function.h"
#include "mini-llvm/ir/GlobalVar.h"
#include "mini-llvm/ir/Instruction.h"
#include "mini-llvm/ir/Instruction/Add.h"
#include "mini-llvm/ir/Instruction/Alloca.h"
#include "mini-llvm/ir/Instruction/And.h"
#include "mini-llvm/ir/Instruction/ASHR.h"
#include "mini-llvm/ir/Instruction/BitCast.h"
#include "mini-llvm/ir/Instruction/Br.h"
#include "mini-llvm/ir/Instruction/Call.h"
#include "mini-llvm/ir/Instruction/CondBr.h"
#include "mini-llvm/ir/Instruction/FAdd.h"
#include "mini-llvm/ir/Instruction/FCmp.h"
#include "mini-llvm/ir/Instruction/FDiv.h"
#include "mini-llvm/ir/Instruction/FMul.h"
#include "mini-llvm/ir/Instruction/FNeg.h"
#include "mini-llvm/ir/Instruction/FPExt.h"
#include "mini-llvm/ir/Instruction/FPToSI.h"
#include "mini-llvm/ir/Instruction/FPToUI.h"
#include "mini-llvm/ir/Instruction/FPTrunc.h"
#include "mini-llvm/ir/Instruction/FRem.h"
#include "mini-llvm/ir/Instruction/FSub.h"
#include "mini-llvm/ir/Instruction/GetElementPtr.h"
#include "mini-llvm/ir/Instruction/ICmp.h"
#include "mini-llvm/ir/Instruction/IndirectCall.h"
#include "mini-llvm/ir/Instruction/IntToPtr.h"
#include "mini-llvm/ir/Instruction/Load.h"
#include "mini-llvm/ir/Instruction/LSHR.h"
#include "mini-llvm/ir/Instruction/Mul.h"
#include "mini-llvm/ir/Instruction/Or.h"
#include "mini-llvm/ir/Instruction/Phi.h"
#include "mini-llvm/ir/Instruction/PtrToInt.h"
#include "mini-llvm/ir/Instruction/Ret.h"
#include "mini-llvm/ir/Instruction/SDiv.h"
#include "mini-llvm/ir/Instruction/Select.h"
#include "mini-llvm/ir/Instruction/SExt.h"
#include "mini-llvm/ir/Instruction/SHL.h"
#include "mini-llvm/ir/Instruction/SIToFP.h"
#include "mini-llvm/ir/Instruction/SRem.h"
#include "mini-llvm/ir/Instruction/Store.h"
#include "mini-llvm/ir/Instruction/Sub.h"
#include "mini-llvm/ir/Instruction/Trunc.h"
#include "mini-llvm/ir/Instruction/UDiv.h"
#include "mini-llvm/ir/Instruction/UIToFP.h"
#include "mini-llvm/ir/Instruction/URem.h"
#include "mini-llvm/ir/Instruction/Xor.h"
#include "mini-llvm/ir/Instruction/ZExt.h"
#include "mini-llvm/ir/InstructionVisitor.h"
#include "mini-llvm/ir/Module.h"
#include "mini-llvm/ir/Type.h"
#include "mini-llvm/ir/Type/ArrayType.h"
#include "mini-llvm/ir/Type/BasicBlockType.h"
#include "mini-llvm/ir/Type/Double.h"
#include "mini-llvm/ir/Type/Float.h"
#include "mini-llvm/ir/Type/FloatingType.h"
#include "mini-llvm/ir/Type/FunctionType.h"
#include "mini-llvm/ir/Type/I1.h"
#include "mini-llvm/ir/Type/I16.h"
#include "mini-llvm/ir/Type/I32.h"
#include "mini-llvm/ir/Type/I64.h"
#include "mini-llvm/ir/Type/I8.h"
#include "mini-llvm/ir/Type/IntegerType.h"
#include "mini-llvm/ir/Type/Ptr.h"
#include "mini-llvm/ir/Type/Void.h"
#include "mini-llvm/ir/Value.h"
#include "mini-llvm/ir_reader/Symbol.h"
#include "mini-llvm/ir_reader/Token.h"
#include "mini-llvm/utils/Memory.h"

using namespace mini_llvm;
using namespace mini_llvm::ir;

using enum Token::Kind;

namespace {

class Dummy final : public Instruction {
public:
    explicit Dummy(std::unique_ptr<Type> type) : type_(std::move(type)) {}

    std::unique_ptr<Type> type() const override {
        return type_->clone();
    }

    std::unordered_set<const UseBase *> operands() const override {
        abort();
    }

    void accept(InstructionVisitor &) override {
        abort();
    }

    void accept(InstructionVisitor &) const override {
        abort();
    }

    std::string format() const override {
        abort();
    }

    std::unique_ptr<Value> clone() const override {
        abort();
    }

private:
    std::unique_ptr<Type> type_;
};

} // namespace

Module Parser::parseModule() {
    Module M;

    std::vector<std::pair<GlobalVar *, std::vector<Token>::const_iterator>> globalVars;
    std::vector<std::pair<Function *, std::vector<Token>::const_iterator>> functions;

    while (cursor_->kind != kEOF) {
        if (cursor_->kind == kAt) {
            bool hasInitializer;
            std::shared_ptr<GlobalVar> G = parseGlobalVarHeader(hasInitializer);
            M.globalVars.append(G);
            if (hasInitializer) {
                globalVars.emplace_back(&*G, cursor_);
                int count = 0;
                do {
                    switch (cursor_->kind) {
                    case kEOF:
                        if (count == 0) {
                            throw ParseException("expected initializer", cursor_);
                        } else {
                            throw ParseException("unclosed '['", cursor_);
                        }

                    case kLeftBracket:
                        ++count;
                        ++cursor_;
                        break;

                    case kRightBracket:
                        --count;
                        ++cursor_;
                        break;

                    case kAt:
                        ++cursor_;
                        if (cursor_->kind == kEOF) {
                            throw ParseException("expected identifier name", cursor_);
                        }
                        ++cursor_;
                        break;

                    default:
                        ++cursor_;
                    }
                } while (count > 0);
            }
        } else if (cursor_->kind == kDefine || cursor_->kind == kDeclare) {
            bool hasBody;
            std::shared_ptr<Function> F = parseFunctionHeader(hasBody);
            M.functions.append(F);
            if (hasBody) {
                functions.emplace_back(&*F, cursor_);
                if (cursor_->kind != kLeftBrace) {
                    throw ParseException("expected '{'", cursor_);
                }
                ++cursor_;
                while (cursor_->kind != kEOF && cursor_->kind != kRightBrace) ++cursor_;
                if (cursor_->kind != kRightBrace) {
                    throw ParseException("expected '}'", cursor_);
                }
                ++cursor_;
            }
        } else {
            throw ParseException("expected '@', 'define' or 'declare'", cursor_);
        }
    }

    for (const auto &[G, cursor] : globalVars) {
        cursor_ = cursor;
        parseGlobalVarInitializer(*G);
    }

    for (const auto &[F, cursor] : functions) {
        cursor_ = cursor;
        parseFunctionBody(*F);
    }

    return M;
}

std::shared_ptr<GlobalVar> Parser::parseGlobalVarHeader(bool &hasInitializer) {
    Symbol symbol = parseSymbol(Symbol::Scope::kGlobal);
    if (symbolTable_.contains(symbol)) {
        throw ParseException("redefinition of global identifier", cursor_);
    }

    if (cursor_->kind != kEqual) {
        throw ParseException("expected '='", cursor_);
    }
    ++cursor_;

    hasInitializer = true;
    if (cursor_->kind == kExternal) {
        hasInitializer = false;
        ++cursor_;
    }

    Linkage linkage = Linkage::kExternal;
    if (cursor_->kind == kInternal) {
        if (!hasInitializer) {
            throw ParseException("invalid linkage", cursor_);
        }
        linkage = Linkage::kInternal;
        ++cursor_;
    } else if (cursor_->kind == kPrivate) {
        if (!hasInitializer) {
            throw ParseException("invalid linkage", cursor_);
        }
        linkage = Linkage::kPrivate;
        ++cursor_;
    }

    if (cursor_->kind != kGlobal) {
        throw ParseException("expected 'global'", cursor_);
    }
    ++cursor_;

    std::unique_ptr<Type> valueType = parseType();

    std::shared_ptr<GlobalVar> G = std::make_shared<GlobalVar>(std::move(valueType), linkage);
    G->setName(symbol.name);
    symbolTable_[symbol] = G;
    return G;
}

void Parser::parseGlobalVarInitializer(GlobalVar &G) {
    G.setInitializer(cast<Constant>(parseValue(*G.valueType())));
}

std::shared_ptr<Function> Parser::parseFunctionHeader(bool &hasBody) {
    switch (cursor_->kind) {
        case kDefine: hasBody = true; break;
        case kDeclare: hasBody = false; break;
        default: throw ParseException("expected 'define' or 'declare'", cursor_);
    }
    ++cursor_;

    Linkage linkage = Linkage::kExternal;
    if (cursor_->kind == kInternal) {
        if (!hasBody) {
            throw ParseException("invalid linkage", cursor_);
        }
        linkage = Linkage::kInternal;
        ++cursor_;
    } else if (cursor_->kind == kPrivate) {
        if (!hasBody) {
            throw ParseException("invalid linkage", cursor_);
        }
        linkage = Linkage::kPrivate;
        ++cursor_;
    }

    std::unique_ptr<Type> returnType = parseType();

    Symbol symbol = parseSymbol(Symbol::Scope::kGlobal);
    if (symbolTable_.contains(symbol)) {
        throw ParseException("redefinition of global identifier", cursor_);
    }

    std::vector<std::unique_ptr<Type>> paramTypes;
    std::vector<std::string> paramNames;
    bool isVarArgs = false;

    if (cursor_->kind != kLeftParen) {
        throw ParseException("expected '('", cursor_);
    }
    ++cursor_;
    if (cursor_->kind != kRightParen) {
        std::unique_ptr<Type> type;
        if (cursor_->kind == kEllipsis) {
            isVarArgs = true;
            ++cursor_;
        } else {
            type = parseType();
            paramTypes.push_back(std::move(type));
            if (hasBody) {
                std::string name = parseSymbol(Symbol::Scope::kLocal).name;
                paramNames.push_back(std::move(name));
            }
        }
        while (!isVarArgs && cursor_->kind == kComma) {
            ++cursor_;
            if (cursor_->kind == kEllipsis) {
                isVarArgs = true;
                ++cursor_;
            } else {
                type = parseType();
                paramTypes.push_back(std::move(type));
                if (hasBody) {
                    std::string name = parseSymbol(Symbol::Scope::kLocal).name;
                    paramNames.push_back(std::move(name));
                }
            }
        }
    }
    if (cursor_->kind != kRightParen) {
        throw ParseException("expected ')'", cursor_);
    }
    ++cursor_;

    std::unique_ptr<FunctionType> type =
        std::make_unique<FunctionType>(std::move(returnType), std::move(paramTypes), isVarArgs);
    std::shared_ptr<Function> F =
        std::make_shared<Function>(std::move(type), linkage);
    F->setName(symbol.name);
    symbolTable_[symbol] = F;

    if (hasBody) {
        for (auto [arg, paramName] : std::views::zip(args(*F), paramNames)) {
            arg.setName(paramName);
        }

        bool completed = false;
        while (!completed) {
            switch (cursor_->kind) {
            case kNoInline:
                F->setAttr(Attribute::kNoInline);
                ++cursor_;
                break;

            case kAlwaysInline:
                F->setAttr(Attribute::kAlwaysInline);
                ++cursor_;
                break;

            default:
                completed = true;
                break;
            }
        }
    }

    return F;
}

void Parser::parseFunctionBody(Function &F) {
    for (Argument &arg : args(F)) {
        Symbol symbol{Symbol::Scope::kLocal, arg.name()};
        if (symbolTable_.contains(symbol)) {
            throw ParseException("redefinition of argument", cursor_);
        }
        symbolTable_[symbol] = share(arg);
    }

    if (cursor_->kind != kLeftBrace) {
        throw ParseException("expected '{'", cursor_);
    }
    ++cursor_;

    auto start = cursor_;
    while (cursor_->kind != kRightBrace) {
        if (cursor_->kind == kName && std::next(cursor_)->kind == kColon) {
            Symbol symbol{Symbol::Scope::kLocal, std::get<std::string>(cursor_->value)};
            if (symbolTable_.contains(symbol)) {
                throw ParseException("redefinition of label", cursor_);
            }
            symbolTable_[symbol] = std::make_shared<BasicBlock>();
        }
        ++cursor_;
    }
    cursor_ = start;

    while (cursor_->kind != kRightBrace) {
        if (!(cursor_->kind == kName && std::next(cursor_)->kind == kColon)) {
            throw ParseException("expected label name", cursor_);
        }
        Symbol symbol{Symbol::Scope::kLocal, std::get<std::string>(cursor_->value)};
        std::shared_ptr<BasicBlock> block = cast<BasicBlock>(symbolTable_[symbol]);
        parseBasicBlock(*block);
        F.append(std::move(block));
    }
    if (F.empty()) {
        throw ParseException("function body cannot be empty", cursor_);
    }
    ++cursor_;

    for (const auto &[symbol, value] : symbolTable_) {
        if (typeid(*value) == typeid(Dummy)) {
            throw ParseException("undefined local identifier", cursor_);
        }
    }

    std::erase_if(symbolTable_, [](const auto &entry) {
        const auto &[symbol, value] = entry;
        return symbol.scope == Symbol::Scope::kLocal;
    });
}

void Parser::parseBasicBlock(BasicBlock &B) {
    if (cursor_->kind != kName) {
        throw ParseException("expected label name", cursor_);
    }
    Symbol symbol{Symbol::Scope::kLocal, std::get<std::string>(cursor_->value)};
    ++cursor_;

    if (cursor_->kind != kColon) {
        throw ParseException("expected ':'", cursor_);
    }
    ++cursor_;

    B.setName(symbol.name);

    while (cursor_->kind != kName && cursor_->kind != kRightBrace) {
        std::shared_ptr<Instruction> I = parseInstruction();
        B.append(I);
    }
}

std::shared_ptr<Instruction> Parser::parseInstruction() {
    std::shared_ptr<Instruction> I;

    if (cursor_->kind == kPercent) {
        Symbol symbol = parseSymbol(Symbol::Scope::kLocal);

        if (cursor_->kind != kEqual) {
            throw ParseException("expected '='", cursor_);
        }
        ++cursor_;

        switch (cursor_->kind) {
            case kFNeg: {
                ++cursor_;
                std::unique_ptr<Type> type = parseType();
                std::shared_ptr<Value> value = parseValue(*type);
                I = std::make_shared<FNeg>(std::move(value));
                break;
            }

            case kAdd:
            case kSub:
            case kFMul:
            case kSDiv:
            case kUDiv:
            case kSRem:
            case kURem:
            case kAnd:
            case kOr:
            case kXor:
            case kSHL:
            case kLSHR:
            case kASHR:
            case kFAdd:
            case kFSub:
            case kMul:
            case kFDiv:
            case kFRem: {
                Token::Kind mnemonic = cursor_->kind;
                ++cursor_;

                std::unique_ptr<Type> type = parseType();
                std::shared_ptr<Value> lhs = parseValue(*type);

                if (cursor_->kind != kComma) {
                    throw ParseException("expected ','", cursor_);
                }
                ++cursor_;

                std::shared_ptr<Value> rhs = parseValue(*type);

                switch (mnemonic) {
                    case kAdd:
                    case kSub:
                    case kMul:
                    case kSDiv:
                    case kUDiv:
                    case kSRem:
                    case kURem:
                    case kAnd:
                    case kOr:
                    case kXor:
                    case kSHL:
                    case kLSHR:
                    case kASHR:
                        if (!dynamic_cast<const IntegerType *>(&*type)) {
                            throw ParseException("must be integer type", cursor_);
                        }
                        break;

                    case kFAdd:
                    case kFSub:
                    case kFMul:
                    case kFDiv:
                    case kFRem:
                        if (!dynamic_cast<const FloatingType *>(&*type)) {
                            throw ParseException("must be floating point type", cursor_);
                        }
                        break;

                    default:
                        abort();
                }

                switch (mnemonic) {
                    case kAdd: I = std::make_shared<Add>(std::move(lhs), std::move(rhs)); break;
                    case kSub: I = std::make_shared<Sub>(std::move(lhs), std::move(rhs)); break;
                    case kMul: I = std::make_shared<Mul>(std::move(lhs), std::move(rhs)); break;
                    case kSDiv: I = std::make_shared<SDiv>(std::move(lhs), std::move(rhs)); break;
                    case kUDiv: I = std::make_shared<UDiv>(std::move(lhs), std::move(rhs)); break;
                    case kSRem: I = std::make_shared<SRem>(std::move(lhs), std::move(rhs)); break;
                    case kURem: I = std::make_shared<URem>(std::move(lhs), std::move(rhs)); break;
                    case kAnd: I = std::make_shared<And>(std::move(lhs), std::move(rhs)); break;
                    case kOr: I = std::make_shared<Or>(std::move(lhs), std::move(rhs)); break;
                    case kXor: I = std::make_shared<Xor>(std::move(lhs), std::move(rhs)); break;
                    case kSHL: I = std::make_shared<SHL>(std::move(lhs), std::move(rhs)); break;
                    case kLSHR: I = std::make_shared<LSHR>(std::move(lhs), std::move(rhs)); break;
                    case kASHR: I = std::make_shared<ASHR>(std::move(lhs), std::move(rhs)); break;
                    case kFAdd: I = std::make_shared<FAdd>(std::move(lhs), std::move(rhs)); break;
                    case kFSub: I = std::make_shared<FSub>(std::move(lhs), std::move(rhs)); break;
                    case kFMul: I = std::make_shared<FMul>(std::move(lhs), std::move(rhs)); break;
                    case kFDiv: I = std::make_shared<FDiv>(std::move(lhs), std::move(rhs)); break;
                    case kFRem: I = std::make_shared<FRem>(std::move(lhs), std::move(rhs)); break;
                    default: abort();
                }

                break;
            }

            case kICmp: {
                ++cursor_;

                ICmp::Condition cond;
                switch (cursor_->kind) {
                    case kEQ: cond = ICmp::Condition::kEQ; break;
                    case kNE: cond = ICmp::Condition::kNE; break;
                    case kSLT: cond = ICmp::Condition::kSLT; break;
                    case kSGT: cond = ICmp::Condition::kSGT; break;
                    case kSLE: cond = ICmp::Condition::kSLE; break;
                    case kSGE: cond = ICmp::Condition::kSGE; break;
                    case kULT: cond = ICmp::Condition::kULT; break;
                    case kUGT: cond = ICmp::Condition::kUGT; break;
                    case kULE: cond = ICmp::Condition::kULE; break;
                    case kUGE: cond = ICmp::Condition::kUGE; break;
                    default: throw ParseException("expected icmp condition", cursor_);
                }
                ++cursor_;

                std::unique_ptr<Type> type = parseType();
                std::shared_ptr<Value> lhs = parseValue(*type);
                if (cursor_->kind != kComma) {
                    throw ParseException("expected ','", cursor_);
                }
                ++cursor_;
                std::shared_ptr<Value> rhs = parseValue(*type);

                I = std::make_shared<ICmp>(cond, std::move(lhs), std::move(rhs));
                break;
            }

            case kFCmp: {
                ++cursor_;

                FCmp::Condition cond;
                switch (cursor_->kind) {
                    case kOEQ: cond = FCmp::Condition::kOEQ; break;
                    case kONE: cond = FCmp::Condition::kONE; break;
                    case kOLT: cond = FCmp::Condition::kOLT; break;
                    case kOGT: cond = FCmp::Condition::kOGT; break;
                    case kOLE: cond = FCmp::Condition::kOLE; break;
                    case kOGE: cond = FCmp::Condition::kOGE; break;
                    default: throw ParseException("expected fcmp condition", cursor_);
                }
                ++cursor_;

                std::unique_ptr<Type> type = parseType();
                std::shared_ptr<Value> lhs = parseValue(*type);
                if (cursor_->kind != kComma) {
                    throw ParseException("expected ','", cursor_);
                }
                ++cursor_;
                std::shared_ptr<Value> rhs = parseValue(*type);

                I = std::make_shared<FCmp>(cond, std::move(lhs), std::move(rhs));
                break;
            }

            case kTrunc:
            case kSExt:
            case kZExt:
            case kFPTrunc:
            case kFPExt:
            case kSIToFP:
            case kUIToFP:
            case kFPToSI:
            case kFPToUI:
            case kPtrToInt:
            case kIntToPtr:
            case kBitCast: {
                Token::Kind mnemonic = cursor_->kind;
                ++cursor_;

                std::unique_ptr<Type> type1 = parseType();
                std::shared_ptr<Value> value = parseValue(*type1);

                if (cursor_->kind != kTo) {
                    throw ParseException("expected 'to'", cursor_);
                }
                ++cursor_;

                std::unique_ptr<Type> type2 = parseType();

                if (mnemonic == kTrunc || mnemonic == kSExt || mnemonic == kZExt || mnemonic == kFPToSI || mnemonic == kFPToUI) {
                    if (!dynamic_cast<const IntegerType *>(&*type2)) {
                        throw ParseException("must be integer type", cursor_);
                    }

                    std::unique_ptr<IntegerType> castType2 = cast<IntegerType>(std::move(type2));

                    switch (mnemonic) {
                        case kTrunc: I = std::make_shared<Trunc>(std::move(value), std::move(castType2)); break;
                        case kSExt: I = std::make_shared<SExt>(std::move(value), std::move(castType2)); break;
                        case kZExt: I = std::make_shared<ZExt>(std::move(value), std::move(castType2)); break;
                        case kFPToSI: I = std::make_shared<FPToSI>(std::move(value), std::move(castType2)); break;
                        case kFPToUI: I = std::make_shared<FPToUI>(std::move(value), std::move(castType2)); break;
                        default: abort();
                    }
                } else if (mnemonic == kFPTrunc || mnemonic == kFPExt || mnemonic == kSIToFP || mnemonic == kUIToFP) {
                    if (!dynamic_cast<const FloatingType *>(&*type2)) {
                        throw ParseException("must be floating point type", cursor_);
                    }

                    std::unique_ptr<FloatingType> castType2 = cast<FloatingType>(std::move(type2));

                    switch (mnemonic) {
                        case kFPTrunc: I = std::make_shared<FPTrunc>(std::move(value), std::move(castType2)); break;
                        case kFPExt: I = std::make_shared<FPExt>(std::move(value), std::move(castType2)); break;
                        case kSIToFP: I = std::make_shared<SIToFP>(std::move(value), std::move(castType2)); break;
                        case kUIToFP: I = std::make_shared<UIToFP>(std::move(value), std::move(castType2)); break;
                        default: abort();
                    }
                } else if (mnemonic == kPtrToInt) {
                    if (*type1 != Ptr()) {
                        throw ParseException("must be ptr", cursor_);
                    }
                    if (!dynamic_cast<const IntegerType *>(&*type2)) {
                        throw ParseException("must be integer type", cursor_);
                    }
                    I = std::make_shared<PtrToInt>(std::move(value), cast<IntegerType>(std::move(type2)));
                } else if (mnemonic == kIntToPtr) {
                    if (!dynamic_cast<const IntegerType *>(&*type1)) {
                        throw ParseException("must be integer type", cursor_);
                    }
                    if (*type2 != Ptr()) {
                        throw ParseException("must be ptr", cursor_);
                    }
                    I = std::make_unique<IntToPtr>(std::move(value));
                } else if (mnemonic == kBitCast) {
                    I = std::make_shared<BitCast>(std::move(value), std::move(type2));
                } else {
                    abort();
                }

                break;
            }

            case kAlloca: {
                ++cursor_;

                std::unique_ptr<Type> allocatedType = parseType();
                I = std::make_shared<Alloca>(std::move(allocatedType));
                break;
            }

            case kLoad: {
                ++cursor_;

                std::unique_ptr<Type> type = parseType();

                if (cursor_->kind != kComma) {
                    throw ParseException("expected ','", cursor_);
                }
                ++cursor_;

                if (*parseType() != Ptr()) {
                    throw ParseException("must be ptr", cursor_);
                }

                std::shared_ptr<Value> ptr = parseValue(Ptr());

                I = std::make_shared<Load>(std::move(type), std::move(ptr));
                break;
            }

            case kSelect: {
                ++cursor_;

                if (*parseType() != I1()) {
                    throw ParseException("must be i1", cursor_);
                }

                std::shared_ptr<Value> cond = parseValue(I1());

                if (cursor_->kind != kComma) {
                    throw ParseException("expected ','", cursor_);
                }
                ++cursor_;

                std::unique_ptr<Type> type = parseType();
                std::shared_ptr<Value> trueValue = parseValue(*type);

                if (cursor_->kind != kComma) {
                    throw ParseException("expected ','", cursor_);
                }
                ++cursor_;

                if (*parseType() != *type) {
                    throw ParseException("both values to select must have same type", cursor_);
                }

                std::shared_ptr<Value> falseValue = parseValue(*type);

                I = std::make_shared<Select>(std::move(cond), std::move(trueValue), std::move(falseValue));
                break;
            }

            case kGetElementPtr: {
                ++cursor_;

                std::unique_ptr<Type> sourceType = parseType();

                if (cursor_->kind != kComma) {
                    throw ParseException("expected ','", cursor_);
                }
                ++cursor_;

                if (*parseType() != Ptr()) {
                    throw ParseException("must be ptr", cursor_);
                }

                std::shared_ptr<Value> ptr = parseValue(Ptr());

                if (cursor_->kind != kComma) {
                    throw ParseException("expected ','", cursor_);
                }
                ++cursor_;

                std::vector<std::shared_ptr<Value>> indices;
                std::unique_ptr<Type> idxType;
                idxType = parseType();
                if (!dynamic_cast<const IntegerType *>(&*idxType)) {
                    throw ParseException("must be integer type", cursor_);
                }
                indices.push_back(parseValue(*idxType));
                while (cursor_->kind == kComma) {
                    ++cursor_;
                    idxType = parseType();
                    if (!dynamic_cast<const IntegerType *>(&*idxType)) {
                        throw ParseException("must be integer type", cursor_);
                    }
                    indices.push_back(parseValue(*idxType));
                }

                I = std::make_shared<GetElementPtr>(std::move(sourceType), std::move(ptr), std::move(indices));

                break;
            }

            case kCall: {
                ++cursor_;

                std::unique_ptr<Type> returnType = parseType();
                if (*returnType == Void()) {
                    throw ParseException("must not be void", cursor_);
                }

                std::shared_ptr<Value> callee = parseValue(Ptr());

                if (cursor_->kind != kLeftParen) {
                    throw ParseException("expected '('", cursor_);
                }
                ++cursor_;

                std::vector<std::shared_ptr<Value>> args;
                if (cursor_->kind != kRightParen) {
                    args.push_back(parseValue(*parseType()));
                    while (cursor_->kind == kComma) {
                        ++cursor_;
                        args.push_back(parseValue(*parseType()));
                    }
                }

                if (cursor_->kind != kRightParen) {
                    throw ParseException("expected ')'", cursor_);
                }
                ++cursor_;

                if (dynamic_cast<const Function *>(&*callee)) {
                    std::shared_ptr<Function> calleeFunction = cast<Function>(callee);

                    if (*calleeFunction->functionType()->returnType() != *returnType) {
                        throw ParseException("inconsistent return type", cursor_);
                    }

                    I =  std::make_shared<Call>(std::move(calleeFunction), std::move(args));
                } else {
                    std::vector<std::unique_ptr<Type>> paramTypes;
                    for (const auto &arg : args) {
                        paramTypes.push_back(arg->type());
                    }

                    std::unique_ptr<FunctionType> functionType =
                        std::make_unique<FunctionType>(std::move(returnType), std::move(paramTypes), false);

                    I = std::make_shared<IndirectCall>(std::move(functionType), std::move(callee), std::move(args));
                }

                break;
            }

            case kPhi: {
                ++cursor_;

                std::shared_ptr<Phi> phi = std::make_shared<Phi>(parseType());

                auto parseIncoming = [this](Phi &phi) {
                    if (cursor_->kind != kLeftBracket) {
                        throw ParseException("expected '['", cursor_);
                    }
                    ++cursor_;

                    std::shared_ptr<Value> value = parseValue(*phi.type());

                    if (cursor_->kind != kComma) {
                        throw ParseException("expected ','", cursor_);
                    }
                    ++cursor_;

                    std::shared_ptr<BasicBlock> B = cast<BasicBlock>(parseValue(BasicBlockType()));

                    if (cursor_->kind != kRightBracket) {
                        throw ParseException("expected ']'", cursor_);
                    }
                    ++cursor_;

                    if (hasIncomingBlock(phi, *B)) {
                        throw ParseException("duplicate incoming block", cursor_);
                    }

                    phi.putIncoming(*B, value);
                };

                parseIncoming(*phi);
                while (cursor_->kind == kComma) {
                    ++cursor_;
                    parseIncoming(*phi);
                }

                I = phi;
                break;
            }

            default:
                throw ParseException("expected instruction mnemonic", cursor_);
        }

        I->setName(symbol.name);

        if (symbolTable_.contains(symbol)) {
            std::shared_ptr<Value> II = symbolTable_[symbol];
            if (typeid(*II) != typeid(Dummy)) {
                throw ParseException("redefinition of local identifier", cursor_);
            }
            if (*II->type() != *I->type()) {
                throw ParseException("inconsistent type", cursor_);
            }
            replaceAllUsesWith(*II, I);
            symbolTable_[symbol] = I;
        } else {
            symbolTable_[symbol] = I;
        }
    } else if (cursor_->kind == kStore) {
        ++cursor_;

        std::unique_ptr<Type> type = parseType();
        std::shared_ptr<Value> value = parseValue(*type);

        if (cursor_->kind != kComma) {
            throw ParseException("expected ','", cursor_);
        }
        ++cursor_;

        if (*parseType() != Ptr()) {
            throw ParseException("must be ptr", cursor_);
        }

        std::shared_ptr<Value> ptr = parseValue(Ptr());

        return std::make_shared<Store>(std::move(value), std::move(ptr));
    } else if (cursor_->kind == kCall) {
        ++cursor_;

        std::unique_ptr<Type> returnType = parseType();
        if (*returnType != Void()) {
            throw ParseException("must be void", cursor_);
        }

        Symbol symbol = parseSymbol(Symbol::Scope::kGlobal);
        if (!symbolTable_.contains(symbol)) {
            throw ParseException("undefined global identifier", cursor_);
        }
        std::shared_ptr<Value> value = symbolTable_[symbol];
        if (!dynamic_cast<const Function *>(&*value)) {
            throw ParseException("identifier must be function", cursor_);
        }
        std::shared_ptr<Function> callee = cast<Function>(value);

        if (*callee->functionType()->returnType() != *returnType) {
            throw ParseException("inconsistent return type", cursor_);
        }

        if (cursor_->kind != kLeftParen) {
            throw ParseException("expected '('", cursor_);
        }
        ++cursor_;

        std::vector<std::shared_ptr<Value>> args;
        if (cursor_->kind != kRightParen) {
            args.push_back(parseValue(*parseType()));
            while (cursor_->kind == kComma) {
                ++cursor_;
                args.push_back(parseValue(*parseType()));
            }
        }

        if (cursor_->kind != kRightParen) {
            throw ParseException("expected ')'", cursor_);
        }
        ++cursor_;

        return std::make_shared<Call>(std::move(callee), std::move(args));
    } else if (cursor_->kind == kBr) {
        ++cursor_;

        std::unique_ptr<Type> type = parseType();

        if (*type == BasicBlockType()) {
            std::shared_ptr<BasicBlock> dest = cast<BasicBlock>(parseValue(BasicBlockType()));
            return std::make_shared<Br>(std::move(dest));
        } else if (*type == I1()) {
            std::shared_ptr<Value> cond = parseValue(I1());

            if (cursor_->kind != kComma) {
                throw ParseException("expected ','", cursor_);
            }
            ++cursor_;

            if (*parseType() != BasicBlockType()) {
                throw ParseException("must be label", cursor_);
            }

            std::shared_ptr<BasicBlock> trueDest = cast<BasicBlock>(parseValue(BasicBlockType()));

            if (cursor_->kind != kComma) {
                throw ParseException("expected ','", cursor_);
            }
            ++cursor_;

            if (*parseType() != BasicBlockType()) {
                throw ParseException("must be label", cursor_);
            }

            std::shared_ptr<BasicBlock> falseDest = cast<BasicBlock>(parseValue(BasicBlockType()));

            return std::make_shared<CondBr>(std::move(cond), std::move(trueDest), std::move(falseDest));
        } else {
            throw ParseException("must be label or i1", cursor_);
        }
    } else if (cursor_->kind == kRet) {
        ++cursor_;

        std::unique_ptr<Type> type = parseType();
        if (*type == Void()) {
            return std::make_shared<Ret>();
        }

        std::shared_ptr<Value> value = parseValue(*type);
        return std::make_shared<Ret>(std::move(value));
    } else {
        throw ParseException("expected local identifier or instruction mnemonic", cursor_);
    }

    return I;
}

std::shared_ptr<Value> Parser::parseValue(const Type &type) {
    switch (cursor_->kind) {
        case kAt:
        case kPercent:
            return parseIdentifier(type);

        case kNumber:
        case kTrue:
        case kFalse:
        case kNull:
        case kZeroInitializer:
        case kLeftBracket:
        case kString:
        case kPoison:
            return parseConstant(type);

        default:
            throw ParseException("expected identifier or constant", cursor_);
    }
}

std::shared_ptr<Value> Parser::parseIdentifier(const Type &type) {
    Symbol symbol = parseSymbol();
    if (symbolTable_.contains(symbol)) {
        std::shared_ptr<Value> value = symbolTable_[symbol];
        if (*value->type() != type) {
            throw ParseException("inconsistent type", cursor_);
        }
        return value;
    }
    if (symbol.scope == Symbol::Scope::kGlobal) {
        throw ParseException("undefined global identifier", cursor_);
    }
    if (type == BasicBlockType()) {
        throw ParseException("undefined label", cursor_);
    }
    return symbolTable_[symbol] = std::make_shared<Dummy>(type.clone());
}

std::unique_ptr<Constant> Parser::parseConstant(const Type &type) {
    if (type == I1()) {
        switch (cursor_->kind) {
            case kTrue: ++cursor_; return std::make_unique<I1Constant>(true);
            case kFalse: ++cursor_; return std::make_unique<I1Constant>(false);
            case kPoison: ++cursor_; return std::make_unique<PoisonValue>(std::make_unique<I1>());
            default: throw ParseException("expected 'true', 'false' or 'poison'", cursor_);
        }
    }
    if (type == I8()) {
        switch (cursor_->kind) {
            case kNumber: {
                int8_t value = static_cast<int8_t>(std::get<int64_t>(cursor_->value));
                ++cursor_;
                return std::make_unique<I8Constant>(value);
            }
            case kPoison:
                ++cursor_;
                return std::make_unique<PoisonValue>(std::make_unique<I8>());
            default:
                throw ParseException("expected number or 'poison'", cursor_);
        }
    }
    if (type == I16()) {
        switch (cursor_->kind) {
            case kNumber: {
                int16_t value = static_cast<int16_t>(std::get<int64_t>(cursor_->value));
                ++cursor_;
                return std::make_unique<I16Constant>(value);
            }
            case kPoison:
                ++cursor_;
                return std::make_unique<PoisonValue>(std::make_unique<I16>());
            default:
                throw ParseException("expected number or 'poison'", cursor_);
        }
    }
    if (type == I32()) {
        switch (cursor_->kind) {
            case kNumber: {
                int32_t value = static_cast<int32_t>(std::get<int64_t>(cursor_->value));
                ++cursor_;
                return std::make_unique<I32Constant>(static_cast<int32_t>(value));
            }
            case kPoison:
                ++cursor_;
                return std::make_unique<PoisonValue>(std::make_unique<I32>());
            default:
                throw ParseException("expected number or 'poison'", cursor_);
        }
    }
    if (type == I64()) {
        switch (cursor_->kind) {
            case kNumber: {
                int64_t value = std::get<int64_t>(cursor_->value);
                ++cursor_;
                return std::make_unique<I64Constant>(value);
            }
            case kPoison:
                ++cursor_;
                return std::make_unique<PoisonValue>(std::make_unique<I64>());
            default:
                throw ParseException("expected number or 'poison'", cursor_);
        }
    }
    if (type == Float()) {
        switch (cursor_->kind) {
            case kNumber: {
                float value = static_cast<float>(std::bit_cast<double>(std::get<int64_t>(cursor_->value)));
                ++cursor_;
                return std::make_unique<FloatConstant>(value);
            }
            case kPoison:
                ++cursor_;
                return std::make_unique<PoisonValue>(std::make_unique<Float>());
            default:
                throw ParseException("expected number or 'poison'", cursor_);
        }
    }
    if (type == Double()) {
        switch (cursor_->kind) {
            case kNumber: {
                double value = std::bit_cast<double>(std::get<int64_t>(cursor_->value));
                ++cursor_;
                return std::make_unique<DoubleConstant>(value);
            }
            case kPoison:
                ++cursor_;
                return std::make_unique<PoisonValue>(std::make_unique<Double>());
            default:
                throw ParseException("expected number or 'poison'", cursor_);
        }
    }
    if (type == Ptr()) {
        switch (cursor_->kind) {
            case kNull:
                ++cursor_;
                return std::make_unique<NullPtrConstant>();
            case kPoison:
                ++cursor_;
                return std::make_unique<PoisonValue>(std::make_unique<Ptr>());
            default:
                throw ParseException("expected 'null' or 'poison'", cursor_);
        }
    }
    if (auto *arrayType = dynamic_cast<const ArrayType *>(&type)) {
        if (cursor_->kind == kPoison) {
            ++cursor_;
            return std::make_unique<PoisonValue>(arrayType->clone());
        }
        if (cursor_->kind == kZeroInitializer) {
            ++cursor_;
            return arrayType->zeroValue();
        }
        std::vector<std::shared_ptr<Constant>> elements;
        if (cursor_->kind == kLeftBracket) {
            ++cursor_;
            if (cursor_->kind != kRightBracket) {
                if (*parseType() != *arrayType->elementType()) {
                    throw ParseException("inconsistent element type", cursor_);
                }
                elements.push_back(parseConstant(*arrayType->elementType()));
                while (cursor_->kind == kComma) {
                    ++cursor_;
                    if (*parseType() != *arrayType->elementType()) {
                        throw ParseException("inconsistent element type", cursor_);
                    }
                    elements.push_back(parseConstant(*arrayType->elementType()));
                }
            }
            if (cursor_->kind != kRightBracket) {
                throw ParseException("expected ']'", cursor_);
            }
            ++cursor_;
            if (elements.size() != arrayType->numElements()) {
                throw ParseException("inconsistent number of elements", cursor_);
            }
        } else if (cursor_->kind == kString) {
            if (ir::I8() != *arrayType->elementType()) {
                throw ParseException("inconsistent element type", cursor_);
            }
            if (std::get<std::vector<int8_t>>(cursor_->value).size() != arrayType->numElements()) {
                throw ParseException("inconsistent number of elements", cursor_);
            }
            for (int8_t element : std::get<std::vector<int8_t>>(cursor_->value)) {
                elements.push_back(std::make_shared<I8Constant>(element));
            }
            ++cursor_;
        } else {
            throw ParseException("expected '[', 'zeroinitializer', 'poison' or string", cursor_);
        }
        return std::make_unique<ArrayConstant>(cast<ArrayType>(arrayType->clone()), std::move(elements));
    }
    throw ParseException("expected constant", cursor_);
}

std::unique_ptr<Type> Parser::parseType() {
    switch (cursor_->kind) {
        case kI1: ++cursor_; return std::make_unique<I1>();
        case kI8: ++cursor_; return std::make_unique<I8>();
        case kI16: ++cursor_; return std::make_unique<I16>();
        case kI32: ++cursor_; return std::make_unique<I32>();
        case kI64: ++cursor_; return std::make_unique<I64>();
        case kFloat: ++cursor_; return std::make_unique<Float>();
        case kDouble: ++cursor_; return std::make_unique<Double>();
        case kPtr: ++cursor_; return std::make_unique<Ptr>();
        case kLabel: ++cursor_; return std::make_unique<BasicBlockType>();
        case kVoid: ++cursor_; return std::make_unique<Void>();

        case kLeftBracket: {
            ++cursor_;
            if (cursor_->kind != kNumber) {
                throw ParseException("expected number", cursor_);
            }
            size_t numElements = static_cast<size_t>(std::get<int64_t>(cursor_->value));
            ++cursor_;
            if (cursor_->kind != kX) {
                throw ParseException("expected 'x'", cursor_);
            }
            ++cursor_;
            std::unique_ptr<Type> elementType = parseType();
            if (cursor_->kind != kRightBracket) {
                throw ParseException("expected ']'", cursor_);
            }
            ++cursor_;
            return std::make_unique<ArrayType>(std::move(elementType), numElements);
        }

        default: throw ParseException("expected type", cursor_);
    }
}

Symbol Parser::parseSymbol(std::optional<Symbol::Scope> scope) {
    Symbol symbol;

    switch (cursor_->kind) {
        case kAt: symbol.scope = Symbol::Scope::kGlobal; break;
        case kPercent: symbol.scope = Symbol::Scope::kLocal; break;
        default: throw ParseException("expected '@' or '%'", cursor_);
    }
    ++cursor_;

    if (cursor_->kind == kName) {
        symbol.name = std::get<std::string>(cursor_->value);
        ++cursor_;
    } else {
        throw ParseException("expected identifier name", cursor_);
    }

    if (scope && symbol.scope != *scope) {
        switch (*scope) {
            case Symbol::Scope::kGlobal: throw ParseException("expected global identifier", cursor_);
            case Symbol::Scope::kLocal: throw ParseException("expected local identifier", cursor_);
        }
    }

    return symbol;
}

Module ir::parseModule(const std::vector<Token> &tokens) {
    return Parser(tokens.begin()).parseModule();
}
