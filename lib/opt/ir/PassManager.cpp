#include "mini-llvm/opt/ir/PassManager.h"

#include <cassert>

#include "mini-llvm/ir/Module.h"
#include "mini-llvm/opt/ir/ModuleTransform.h"
#include "mini-llvm/opt/ir/passes/AlgebraicSimplification.h"
#include "mini-llvm/opt/ir/passes/BasicBlockMerging.h"
#include "mini-llvm/opt/ir/passes/BranchSimplification.h"
#include "mini-llvm/opt/ir/passes/ConstantFolding.h"
#include "mini-llvm/opt/ir/passes/DeadCodeElimination.h"
#include "mini-llvm/opt/ir/passes/FunctionInlining.h"
#include "mini-llvm/opt/ir/passes/GlobalValueNumbering.h"
#include "mini-llvm/opt/ir/passes/JumpThreading.h"
#include "mini-llvm/opt/ir/passes/Mem2Reg.h"
#include "mini-llvm/opt/ir/passes/PoisonPropagation.h"
#include "mini-llvm/opt/ir/passes/StrengthReduction.h"
#include "mini-llvm/opt/ir/passes/UnreachableBlockElimination.h"
#include "mini-llvm/opt/ir/Verify.h"

using namespace mini_llvm::ir;

void PassManager::run(Module &M) const {
    assert(verify(M));

    Mem2Reg pass0;

    pass0.runOnModule(M);
    assert(verify(M));

    bool changed;
    do {
        changed = false;

        DeadCodeElimination         pass1;
        BranchSimplification        pass2;
        JumpThreading               pass3;
        BasicBlockMerging           pass4;
        UnreachableBlockElimination pass5;
        AlgebraicSimplification     pass6;
        ConstantFolding             pass7;
        PoisonPropagation           pass8;
        GlobalValueNumbering        pass9;
        StrengthReduction           pass10(3);
        FunctionInlining            pass11;

        ModuleTransform *passes[] = {
            &pass1,
            &pass2,
            &pass3,
            &pass4,
            &pass5,
            &pass6,
            &pass7,
            &pass8,
            &pass9,
            &pass10,
            &pass11,
        };

        for (ModuleTransform *pass : passes) {
            changed |= pass->runOnModule(M);
            assert(verify(M));
        }
    } while (changed);
}
