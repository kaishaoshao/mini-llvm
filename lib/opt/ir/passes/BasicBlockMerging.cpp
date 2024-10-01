#include "mini-llvm/opt/ir/passes/BasicBlockMerging.h"

#include <cassert>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mini-llvm/ir/BasicBlock.h"
#include "mini-llvm/ir/Function.h"
#include "mini-llvm/ir/Instruction.h"
#include "mini-llvm/ir/Instruction/Br.h"
#include "mini-llvm/ir/Instruction/Phi.h"
#include "mini-llvm/ir/Use.h"
#include "mini-llvm/ir/Value.h"
#include "mini-llvm/utils/Memory.h"

using namespace mini_llvm::ir;

namespace {

bool canMergeWithSuccessor(const BasicBlock &B, const std::unordered_map<const BasicBlock *, int> &numPredecessors) {
    if (B.empty()) {
        return false;
    }
    if (!dynamic_cast<const Br *>(&B.back())) {
        return false;
    }
    const BasicBlock &succ = *static_cast<const Br *>(&B.back())->dest();
    if (numPredecessors.at(&succ) > 1) {
        return false;
    }
    for (const Instruction &I : succ) {
        if (dynamic_cast<const Phi *>(&I)) {
            return false;
        }
    }
    return true;
}

} // namespace

bool BasicBlockMerging::runOnFunction(Function &F) {
    bool changed = false;

    std::unordered_map<const BasicBlock *, int> numPredecessors;
    for (const BasicBlock &B : F) {
        numPredecessors.emplace(&B, 0);
    }
    for (const BasicBlock &B : F) {
        for (const BasicBlock *succ : successors(B)) {
            ++numPredecessors[succ];
        }
    }

    for (BasicBlock &B : F) {
        if (numPredecessors[&B] != 1) {
            while (canMergeWithSuccessor(B, numPredecessors)) {
                BasicBlock &succ = *static_cast<Br *>(&B.back())->dest();
                removeFromParent(B.back());
                while (!succ.empty()) {
                    std::shared_ptr<Instruction> I = share(succ.front());
                    removeFromParent(*I);
                    B.append(std::move(I));
                }
#ifndef NDEBUG
                for (const UseBase &use : uses(succ)) {
                    assert(dynamic_cast<const Phi *>(use.user()));
                }
#endif
                replaceAllUsesWith(succ, weaken(B));
                changed = true;
            }
        }
    }

    std::vector<const BasicBlock *> remove;
    for (const BasicBlock &B : F) {
        if (B.empty()) {
            remove.push_back(&B);
        }
    }
    for (const BasicBlock *B : remove) {
        removeFromParent(*B);
        changed = true;
    }

    return changed;
}
