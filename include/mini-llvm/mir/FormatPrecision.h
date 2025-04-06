#pragma once

#include <cstdlib>

#include "mini-llvm/common/Precision.h"

namespace mini_llvm::mir {

inline constexpr const char *specifier(Precision precision) {
    using enum Precision;
    switch (precision) {
        case kSingle: return "s";
        case kDouble: return "d";
        default: abort();
    }
}

} // namespace mini_llvm::mir
