#include "mini-llvm/opt/mir/passes/TailDuplication.h"

#include <memory>
#include <unordered_map>
#include <vector>

#include "mini-llvm/mir/BasicBlock.h"
#include "mini-llvm/mir/Function.h"
#include "mini-llvm/mir/Instruction.h"
#include "mini-llvm/mir/Instruction/Br.h"

using namespace mini_llvm::mir;

bool TailDuplication::runOnFunction(Function &F) {
    bool changed = false;

    bool changed2;
    do {
        changed2 = false;

        std::unordered_map<BasicBlock *, std::vector<BasicBlock *>> predecessors;

        for (BasicBlock &B : F) {
            for (BasicBlock *succ : successors(B)) {
                predecessors[succ].push_back(&B);
            }
        }

        for (BasicBlock &B : F) {
            if (!successors(B).empty() && B.size() > 1 && B.size() <= threshold_) {
                for (BasicBlock *pred : predecessors[&B]) {
                    if (dynamic_cast<const Br *>(&pred->back())) {
                        std::vector<std::unique_ptr<Instruction>> tmp;
                        for (Instruction &I : B) {
                            tmp.push_back(I.clone());
                        }
                        pred->removeLast();
                        for (auto &I : tmp) {
                            pred->append(std::move(I));
                        }
                        changed2 = true;
                    }
                }
            }
        }

        changed |= changed2;
    } while (changed2);

    return changed;
}
