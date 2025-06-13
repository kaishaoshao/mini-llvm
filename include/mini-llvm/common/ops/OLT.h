#pragma once

#include <cmath>
#include <concepts>

namespace mini_llvm::ops {

struct OLT {
    template <typename T>
        requires std::floating_point<T>
    bool operator()(T x, T y) const noexcept {
        return std::isless(x, y);
    }
};

} // namespace mini_llvm::ops
