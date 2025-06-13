#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>

#include "mini-llvm/ir/Constant.h"
#include "mini-llvm/ir/Type.h"
#include "mini-llvm/ir/Type/Ptr.h"
#include "mini-llvm/ir/Value.h"
#include "mini-llvm/utils/ToString.h"

namespace mini_llvm::ir {

class GlobalValue : public Constant {
public:
    std::unique_ptr<Type> type() const override {
        return std::make_unique<Ptr>();
    }

    std::string formatAsOperand() const override {
        return "@" + (!name().empty() ? name() : "_" + toString(reinterpret_cast<uintptr_t>(this), 62));
    }

    std::unique_ptr<Value> clone() const override {
        abort();
    }

protected:
    bool equals(const Constant &) const override {
        abort();
    }
};

} // namespace mini_llvm::ir
