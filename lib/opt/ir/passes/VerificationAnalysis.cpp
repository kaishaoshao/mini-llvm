#include "mini-llvm/opt/ir/passes/VerificationAnalysis.h"

#include <iterator>
#include <memory>
#include <queue>
#include <unordered_set>

#include "mini-llvm/ir/BasicBlock.h"
#include "mini-llvm/ir/Function.h"
#include "mini-llvm/ir/Instruction.h"
#include "mini-llvm/ir/Instruction/BinaryFloatingOperator.h"
#include "mini-llvm/ir/Instruction/BinaryIntegerOperator.h"
#include "mini-llvm/ir/Instruction/FPExt.h"
#include "mini-llvm/ir/Instruction/FPTrunc.h"
#include "mini-llvm/ir/Instruction/Phi.h"
#include "mini-llvm/ir/Instruction/Ret.h"
#include "mini-llvm/ir/Instruction/Select.h"
#include "mini-llvm/ir/Instruction/SExt.h"
#include "mini-llvm/ir/Instruction/Terminator.h"
#include "mini-llvm/ir/Instruction/Trunc.h"
#include "mini-llvm/ir/Instruction/ZExt.h"
#include "mini-llvm/ir/Module.h"
#include "mini-llvm/ir/Type/FunctionType.h"
#include "mini-llvm/ir/Type/Ptr.h"
#include "mini-llvm/ir/Use.h"
#include "mini-llvm/opt/ir/passes/DominatorTreeAnalysis.h"

using namespace mini_llvm;
using namespace mini_llvm::ir;

void VerificationAnalysis::runOnFunction(const Function &F) {
    isValid_ = true;

    for (const BasicBlock &B : F) {
        for (const Instruction &I : B) {
            for (const UseBase *op : I.operands()) {
                if (op->expired()) {
                    isValid_ = false;
                    return;
                }
            }
        }
    }

    for (const BasicBlock &B : F) {
        if (B.empty()) {
            isValid_ = false;
            return;
        }
        if (!dynamic_cast<const Terminator *>(&B.back())) {
            isValid_ = false;
            return;
        }
        for (auto i = B.begin(), e = std::prev(B.end()); i != e; ++i) {
            if (dynamic_cast<const Terminator *>(&*i)) {
                isValid_ = false;
                return;
            }
        }
    }

    for (const BasicBlock &B : F) {
        if (auto *ret = dynamic_cast<const Ret *>(&B.back())) {
            if (*ret->value()->type() != *F.functionType()->returnType()) {
                isValid_ = false;
                return;
            }
        }
    }

    if (!hasNPredecessors(F.entry(), 0)) {
        isValid_ = false;
        return;
    }

    for (const BasicBlock &B : F) {
        for (const Instruction &I : B) {
            if (auto *phi = dynamic_cast<const Phi *>(&I)) {
                if (incomingBlocks(*phi) != predecessors(B)) {
                    isValid_ = false;
                    return;
                }
            }
        }
    }

    for (const BasicBlock &B : F) {
        for (const Instruction &I : B) {
            if (auto *op = dynamic_cast<const BinaryIntegerOperator *>(&I)) {
                if (*op->lhs()->type() != *op->rhs()->type()) {
                    isValid_ = false;
                    return;
                }
                if (*op->opType() == Ptr()) {
                    isValid_ = false;
                    return;
                }
            }
            if (auto *op = dynamic_cast<const BinaryFloatingOperator *>(&I)) {
                if (*op->lhs()->type() != *op->rhs()->type()) {
                    isValid_ = false;
                    return;
                }
                if (*op->opType() == Ptr()) {
                    isValid_ = false;
                    return;
                }
            }
            if (auto *trunc = dynamic_cast<const Trunc *>(&I)) {
                if (*trunc->value()->type() == Ptr()) {
                    isValid_ = false;
                    return;
                }
                if (*trunc->type() == Ptr()) {
                    isValid_ = false;
                    return;
                }
                if (trunc->type()->sizeInBits() >= trunc->value()->type()->sizeInBits()) {
                    isValid_ = false;
                    return;
                }
            }
            if (auto *sext = dynamic_cast<const SExt *>(&I)) {
                if (*sext->value()->type() == Ptr()) {
                    isValid_ = false;
                    return;
                }
                if (*sext->type() == Ptr()) {
                    isValid_ = false;
                    return;
                }
                if (sext->type()->sizeInBits() <= sext->value()->type()->sizeInBits()) {
                    isValid_ = false;
                    return;
                }
            }
            if (auto *zext = dynamic_cast<const ZExt *>(&I)) {
                if (*zext->value()->type() == Ptr()) {
                    isValid_ = false;
                    return;
                }
                if (*zext->type() == Ptr()) {
                    isValid_ = false;
                    return;
                }
                if (zext->type()->sizeInBits() <= zext->value()->type()->sizeInBits()) {
                    isValid_ = false;
                    return;
                }
            }
            if (auto *fptrunc = dynamic_cast<const FPTrunc *>(&I)) {
                if (fptrunc->type()->sizeInBits() >= fptrunc->value()->type()->sizeInBits()) {
                    isValid_ = false;
                    return;
                }
            }
            if (auto *fpext = dynamic_cast<const FPExt *>(&I)) {
                if (fpext->type()->sizeInBits() <= fpext->value()->type()->sizeInBits()) {
                    isValid_ = false;
                    return;
                }
            }
            if (auto *select = dynamic_cast<const Select *>(&I)) {
                if (*select->trueValue()->type() != *select->falseValue()->type()) {
                    isValid_ = false;
                    return;
                }
            }
        }
    }

    std::unordered_set<const BasicBlock *> S;
    std::queue<const BasicBlock *> Q;
    S.insert(&F.entry());
    Q.push(&F.entry());
    while (!Q.empty()) {
        const BasicBlock *u = Q.front();
        Q.pop();
        for (const BasicBlock *v : successors(*u)) {
            if (!S.contains(v)) {
                S.insert(v);
                Q.push(v);
            }
        }
    }

    DominatorTreeAnalysis domTree;
    domTree.runOnFunction(F);

    for (const BasicBlock *B : S) {
        for (const Instruction &I : *B) {
            for (const UseBase &use : uses(I)) {
                if (auto *II = dynamic_cast<const Instruction *>(use.user())) {
                    if (!S.contains(II->parent())) {
                        isValid_ = false;
                        return;
                    }
                    if (!dynamic_cast<const Phi *>(II) && !domTree.dominates(I, *II)) {
                        isValid_ = false;
                        return;
                    }
                }
            }
        }
    }

    for (const BasicBlock &B : F) {
        for (const Instruction &I : B) {
            if (!dynamic_cast<const Phi *>(&I)) {
                for (const UseBase *op : I.operands()) {
                    if (&**op == &I) {
                        isValid_ = false;
                        return;
                    }
                }
            }
        }
    }
}

void VerificationAnalysis::runOnModule(const Module &M) {
    isValid_ = true;
    for (const Function &F : M.functions) {
        if (!F.empty()) {
            runOnFunction(F);
            if (!isValid_) {
                return;
            }
        }
    }
}

bool ir::verifyFunction(const Function &F) {
    VerificationAnalysis verify;
    verify.runOnFunction(F);
    return verify.isValid();
}

bool ir::verifyModule(const Module &M) {
    VerificationAnalysis verify;
    verify.runOnModule(M);
    return verify.isValid();
}
