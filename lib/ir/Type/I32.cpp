#include "mini-llvm/ir/Type/I32.h"

#include <cstdint>
#include <memory>

#include "mini-llvm/common/ops/Trunc.h"
#include "mini-llvm/ir/Constant.h"
#include "mini-llvm/ir/Constant/I32Constant.h"
#include "mini-llvm/ir/Type.h"

using namespace mini_llvm;
using namespace mini_llvm::ir;

std::unique_ptr<Constant> I32::zeroValue() const {
    return std::make_unique<I32Constant>(0);
}

std::unique_ptr<Constant> I32::constant(int64_t value) const {
    return std::make_unique<I32Constant>(ops::Trunc<int32_t>()(value));
}
