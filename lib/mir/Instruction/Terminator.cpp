#include "mini-llvm/mir/Instruction/Terminator.h"

#include <unordered_set>
#include <utility>

using namespace mini_llvm::mir;

std::unordered_set<BasicBlockOperand *> Terminator::blockOps() {
    std::unordered_set<BasicBlockOperand *> successors;
    for (const BasicBlockOperand *op : std::as_const(*this).blockOps()) {
        successors.insert(const_cast<BasicBlockOperand *>(op));
    }
    return successors;
}
