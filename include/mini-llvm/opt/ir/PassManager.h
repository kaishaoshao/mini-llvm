#pragma once

#include "mini-llvm/ir/Module.h"

namespace mini_llvm::ir {

class PassManager {
public:
    void run(Module &M) const;
};

} // namespace mini_llvm::ir
