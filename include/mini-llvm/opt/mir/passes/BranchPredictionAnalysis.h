#pragma once

#include <memory>

#include "mini-llvm/mir/BasicBlock.h"
#include "mini-llvm/mir/Function.h"
#include "mini-llvm/opt/mir/FunctionAnalysis.h"

namespace mini_llvm::mir {

class BranchPredictionAnalysis final : public FunctionAnalysis {
public:
    BranchPredictionAnalysis();
    ~BranchPredictionAnalysis() override;
    void runOnFunction(const Function &F) override;
    bool predict(const BasicBlock &B, const BasicBlock &succ) const;

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
};

} // namespace mini_llvm::mir
