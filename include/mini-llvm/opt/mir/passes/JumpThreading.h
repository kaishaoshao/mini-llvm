#pragma once

#include "mini-llvm/mir/Function.h"
#include "mini-llvm/opt/mir/FunctionTransform.h"

namespace mini_llvm::mir {

class JumpThreading final : public FunctionTransform {
public:
    bool runOnFunction(Function &F) override;
};

} // namespace mini_llvm::mir
