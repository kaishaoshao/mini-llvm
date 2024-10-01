#pragma once

#include "mini-llvm/ir/Function.h"
#include "mini-llvm/ir/Module.h"
#include "mini-llvm/opt/ir/FunctionAnalysis.h"
#include "mini-llvm/opt/ir/ModuleAnalysis.h"

namespace mini_llvm::ir {

class VerificationAnalysis final : public FunctionAnalysis, public ModuleAnalysis {
public:
    void runOnFunction(const Function &F) override;
    void runOnModule(const Module &M) override;

    bool isValid() const {
        return isValid_;
    }

private:
    bool isValid_;
};

bool verifyFunction(const Function &F);
bool verifyModule(const Module &M);

} // namespace mini_llvm::ir
