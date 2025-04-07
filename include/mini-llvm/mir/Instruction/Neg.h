#pragma once

#include <memory>
#include <utility>

#include "mini-llvm/common/ExtensionMode.h"
#include "mini-llvm/mir/Instruction.h"
#include "mini-llvm/mir/Instruction/UnaryOperator.h"
#include "mini-llvm/mir/InstructionVisitor.h"
#include "mini-llvm/mir/Register.h"
#include "mini-llvm/utils/Memory.h"

namespace mini_llvm::mir {

class Neg : public UnaryOperator {
public:
    Neg(int width,
        std::shared_ptr<Register> dst,
        std::shared_ptr<Register> src,
        ExtensionMode extensionMode = ExtensionMode::kNo)
        : UnaryOperator(width, std::move(dst), std::move(src), extensionMode) {}

    bool hasSideEffects() const override {
        return false;
    }

    std::unique_ptr<Instruction> clone() const override {
        return std::make_unique<Neg>(
            width(), share(*dst()), share(*src()), extensionMode());
    }

    void accept(InstructionVisitor &visitor) override {
        visitor.visitNeg(*this);
    }

    void accept(InstructionVisitor &visitor) const override {
        visitor.visitNeg(*this);
    }

protected:
    const char *mnemonic() const override {
        return "NEG";
    }
};

} // namespace mini_llvm::mir
