#include "mini-llvm/targets/riscv/opt/mir/RISCVPassManager.h"

#include "mini-llvm/mir/Module.h"
#include "mini-llvm/opt/mir/passes/BasicBlockMerging.h"
#include "mini-llvm/opt/mir/passes/CopyPropagation.h"
#include "mini-llvm/opt/mir/passes/DeadCodeElimination.h"
#include "mini-llvm/opt/mir/passes/JumpThreading.h"
#include "mini-llvm/opt/mir/passes/NullOperationElimination.h"
#include "mini-llvm/opt/mir/passes/RegisterReuse.h"
#include "mini-llvm/opt/mir/passes/StackOffsetEvaluation.h"
#include "mini-llvm/opt/mir/passes/TailDuplication.h"
#include "mini-llvm/opt/mir/passes/UnreachableBlockElimination.h"
#include "mini-llvm/opt/mir/passes/ZeroRegisterReplacement.h"
#include "mini-llvm/targets/riscv/mir/RISCVRegister.h"
#include "mini-llvm/targets/riscv/opt/mir/passes/RISCVConstantPropagation.h"

using namespace mini_llvm::mir;
using namespace mini_llvm::mir::riscv;

void RISCVPassManager::runBeforeRegisterAllocation(Module &M) const {
    bool changed;
    do {
        changed = false;

        NullOperationElimination    pass1(8);
        RISCVConstantPropagation    pass2;
        CopyPropagation             pass3;
        RegisterReuse               pass4;
        ZeroRegisterReplacement     pass5(x0());
        DeadCodeElimination         pass6;
        JumpThreading               pass7;
        BasicBlockMerging           pass8;
        TailDuplication             pass9(8);
        UnreachableBlockElimination pass10;

        changed |= pass1.runOnModule(M);
        changed |= pass2.runOnModule(M);
        changed |= pass3.runOnModule(M);
        changed |= pass4.runOnModule(M);
        changed |= pass5.runOnModule(M);
        changed |= pass6.runOnModule(M);
        changed |= pass7.runOnModule(M);
        changed |= pass8.runOnModule(M);
        changed |= pass9.runOnModule(M);
        changed |= pass10.runOnModule(M);
    } while (changed);
}

void RISCVPassManager::runAfterRegisterAllocation(Module &M) const {
    StackOffsetEvaluation pass0;
    pass0.runOnModule(M);

    bool changed;
    do {
        changed = false;

        NullOperationElimination    pass1(8);
        RISCVConstantPropagation    pass2;
        CopyPropagation             pass3;
        RegisterReuse               pass4;
        ZeroRegisterReplacement     pass5(x0());
        DeadCodeElimination         pass6;
        JumpThreading               pass7;
        BasicBlockMerging           pass8;
        TailDuplication             pass9(8);
        UnreachableBlockElimination pass10;

        changed |= pass1.runOnModule(M);
        changed |= pass2.runOnModule(M);
        changed |= pass3.runOnModule(M);
        changed |= pass4.runOnModule(M);
        changed |= pass5.runOnModule(M);
        changed |= pass6.runOnModule(M);
        changed |= pass7.runOnModule(M);
        changed |= pass8.runOnModule(M);
        changed |= pass9.runOnModule(M);
        changed |= pass10.runOnModule(M);
    } while (changed);
}
