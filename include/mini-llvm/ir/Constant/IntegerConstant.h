#pragma once

#include <cstdint>

#include "mini-llvm/ir/Constant.h"
#include "mini-llvm/ir/Type.h"

namespace mini_llvm::ir {

class IntegerConstant : public Constant {
public:
    virtual int64_t signExtendedValue() const = 0;
    virtual int64_t zeroExtendedValue() const = 0;
};

} // namespace mini_llvm::ir
