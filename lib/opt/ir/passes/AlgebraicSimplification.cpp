#include "mini-llvm/opt/ir/passes/AlgebraicSimplification.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "mini-llvm/ir/BasicBlock.h"
#include "mini-llvm/ir/Constant/I1Constant.h"
#include "mini-llvm/ir/Constant/IntegerConstant.h"
#include "mini-llvm/ir/Constant/PoisonValue.h"
#include "mini-llvm/ir/Function.h"
#include "mini-llvm/ir/Instruction.h"
#include "mini-llvm/ir/Instruction/Add.h"
#include "mini-llvm/ir/Instruction/And.h"
#include "mini-llvm/ir/Instruction/ASHR.h"
#include "mini-llvm/ir/Instruction/BinaryIntegerArithmeticOperator.h"
#include "mini-llvm/ir/Instruction/ICmp.h"
#include "mini-llvm/ir/Instruction/LSHR.h"
#include "mini-llvm/ir/Instruction/Mul.h"
#include "mini-llvm/ir/Instruction/Or.h"
#include "mini-llvm/ir/Instruction/Phi.h"
#include "mini-llvm/ir/Instruction/SDiv.h"
#include "mini-llvm/ir/Instruction/Select.h"
#include "mini-llvm/ir/Instruction/SHL.h"
#include "mini-llvm/ir/Instruction/SRem.h"
#include "mini-llvm/ir/Instruction/Sub.h"
#include "mini-llvm/ir/Instruction/UDiv.h"
#include "mini-llvm/ir/Instruction/URem.h"
#include "mini-llvm/ir/Instruction/Xor.h"
#include "mini-llvm/ir/Value.h"
#include "mini-llvm/opt/ir/passes/DominatorTreeAnalysis.h"
#include "mini-llvm/utils/Memory.h"

using namespace mini_llvm;
using namespace mini_llvm::ir;

namespace {

bool isUnchanged(const BinaryIntegerArithmeticOperator &op) {
    int64_t rhs = static_cast<const IntegerConstant *>(&*op.rhs())->signExtendedValue();

    return (dynamic_cast<const Add *>(&op) && rhs == 0)
        || (dynamic_cast<const Sub *>(&op) && rhs == 0)
        || (dynamic_cast<const Or *>(&op) && rhs == 0)
        || (dynamic_cast<const Xor *>(&op) && rhs == 0)
        || (dynamic_cast<const SHL *>(&op) && rhs == 0)
        || (dynamic_cast<const LSHR *>(&op) && rhs == 0)
        || (dynamic_cast<const ASHR *>(&op) && rhs == 0)
        || (dynamic_cast<const Mul *>(&op) && rhs == 1)
        || (dynamic_cast<const SDiv *>(&op) && rhs == 1)
        || (dynamic_cast<const UDiv *>(&op) && rhs == 1)
        || (dynamic_cast<const And *>(&op) && rhs == -1);
}

bool isZero(const BinaryIntegerArithmeticOperator &op) {
    int64_t rhs = static_cast<const IntegerConstant *>(&*op.rhs())->signExtendedValue();

    return (dynamic_cast<const Mul *>(&op) && rhs == 0)
        || (dynamic_cast<const And *>(&op) && rhs == 0)
        || (dynamic_cast<const SRem *>(&op) && rhs == 1)
        || (dynamic_cast<const URem *>(&op) && rhs == 1);
}

bool isPoison(const BinaryIntegerArithmeticOperator &op) {
    int64_t rhs = static_cast<const IntegerConstant *>(&*op.rhs())->signExtendedValue();

    return (dynamic_cast<const SDiv *>(&op) && rhs == 0)
        || (dynamic_cast<const UDiv *>(&op) && rhs == 0)
        || (dynamic_cast<const SRem *>(&op) && rhs == 0)
        || (dynamic_cast<const URem *>(&op) && rhs == 0)
        || (dynamic_cast<const SHL *>(&op) && rhs >= op.type()->sizeInBits())
        || (dynamic_cast<const LSHR *>(&op) && rhs >= op.type()->sizeInBits())
        || (dynamic_cast<const ASHR *>(&op) && rhs >= op.type()->sizeInBits());
}

void dfs(const DominatorTreeNode *node, bool &changed) {
    std::vector<const Instruction *> remove;

    for (const Instruction &I : *node->block) {
        if (auto *op = dynamic_cast<const BinaryIntegerArithmeticOperator *>(&I)) {
            const Value *lhs = &*op->lhs(),
                        *rhs = &*op->rhs();

            if (!dynamic_cast<const IntegerConstant *>(lhs) && dynamic_cast<const IntegerConstant *>(rhs)) {
                if (isUnchanged(*op)) {
                    changed |= replaceAllUsesWith(*op, share(*const_cast<Value *>(lhs)));
                    remove.push_back(op);
                    continue;
                }

                if (isZero(*op)) {
                    changed |= replaceAllUsesWith(*op, op->type()->constant(0));
                    remove.push_back(op);
                    continue;
                }

                if (isPoison(*op)) {
                    changed |= replaceAllUsesWith(*op, std::make_shared<PoisonValue>(op->type()));
                    remove.push_back(op);
                    continue;
                }

                continue;
            }

            if (!dynamic_cast<const IntegerConstant *>(lhs) && lhs == rhs) {
                if (dynamic_cast<const Add *>(op)) {
                    Mul &mul = addToParent(*op, std::make_shared<Mul>(share(*const_cast<Value *>(lhs)), op->lhs()->type()->constant(2)));
                    replaceAllUsesWith(*op, share(mul));
                    changed = true;
                    remove.push_back(op);
                    continue;
                }

                if (dynamic_cast<const And *>(op) || dynamic_cast<const Or *>(op)) {
                    changed |= replaceAllUsesWith(*op, share(*const_cast<Value *>(lhs)));
                    remove.push_back(op);
                    continue;
                }

                if (dynamic_cast<const Sub *>(op)
                        || dynamic_cast<const SRem *>(op)
                        || dynamic_cast<const URem *>(op)
                        || dynamic_cast<const Xor *>(op)) {
                    changed |= replaceAllUsesWith(*op, op->type()->constant(0));
                    remove.push_back(op);
                    continue;
                }

                if (dynamic_cast<const SDiv *>(op) || dynamic_cast<const UDiv *>(op)) {
                    changed |= replaceAllUsesWith(*op, op->type()->constant(1));
                    remove.push_back(op);
                    continue;
                }

                continue;
            }

            continue;
        }

        if (auto *icmp = dynamic_cast<const ICmp *>(&I)) {
            if (&*icmp->lhs() == &*icmp->rhs()) {
                bool result;
                switch (icmp->cond()) {
                    case ICmp::Condition::kEQ:
                    case ICmp::Condition::kSLE:
                    case ICmp::Condition::kSGE:
                    case ICmp::Condition::kULE:
                    case ICmp::Condition::kUGE:
                        result = true;
                        break;

                    case ICmp::Condition::kNE:
                    case ICmp::Condition::kSLT:
                    case ICmp::Condition::kSGT:
                    case ICmp::Condition::kULT:
                    case ICmp::Condition::kUGT:
                        result = false;
                        break;
                }
                changed |= replaceAllUsesWith(*icmp, std::make_shared<I1Constant>(result));
                continue;
            }

            continue;
        }

        if (auto *select = dynamic_cast<const Select *>(&I)) {
            const Value *trueValue = &*select->trueValue(),
                        *falseValue = &*select->falseValue();

            if (!dynamic_cast<const IntegerConstant *>(trueValue) && trueValue == falseValue) {
                changed |= replaceAllUsesWith(*select, share(*const_cast<Value *>(trueValue)));
                remove.push_back(select);
                continue;
            }

            continue;
        }

        if (auto *phi = dynamic_cast<const Phi *>(&I)) {
            if (phi->incoming_size() == 1) {
                changed |= replaceAllUsesWith(*phi, share(*phi->incoming_begin()->value));
                remove.push_back(phi);
                continue;
            }

            continue;
        }
    }

    for (const Instruction *I : remove) {
        removeFromParent(*I);
        changed = true;
    }

    for (const DominatorTreeNode *child : node->children) {
        dfs(child, changed);
    }
}

} // namespace

bool AlgebraicSimplification::runOnFunction(Function &F) {
    bool changed = false;

    for (BasicBlock &B : F) {
        for (Instruction &I : B) {
            if (auto *op = dynamic_cast<BinaryIntegerArithmeticOperator *>(&I)) {
                if (op->isCommutative()
                        && dynamic_cast<const IntegerConstant *>(&*op->lhs())
                        && !dynamic_cast<const IntegerConstant *>(&*op->rhs())) {
                    std::shared_ptr<Value> lhs = share(*op->lhs()),
                                           rhs = share(*op->rhs());
                    op->lhs().set(std::move(rhs));
                    op->rhs().set(std::move(lhs));
                    changed = true;
                }
            }
        }
    }

    DominatorTreeAnalysis domTree;
    domTree.runOnFunction(F);

    dfs(domTree.node(F.entry()), changed);

    return changed;
}
