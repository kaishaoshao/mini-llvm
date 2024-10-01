#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <typeinfo>

#include "mini-llvm/ir/Constant.h"
#include "mini-llvm/ir/Type.h"
#include "mini-llvm/ir/Type/IntegerType.h"
#include "mini-llvm/ir/TypeVisitor.h"

namespace mini_llvm::ir {

class I1 final : public IntegerType {
public:
    int size() const override {
        abort();
    }

    int alignment() const override {
        abort();
    }

    int sizeInBits() const override {
        return 1;
    }

    int alignmentInBits() const override {
        abort();
    }

    std::unique_ptr<Constant> zeroValue() const override;

    std::string format() const override {
        return "i1";
    }

    std::unique_ptr<Type> clone() const override {
        return std::make_unique<I1>();
    }

    void accept(TypeVisitor &visitor) override {
        visitor.visitI1(*this);
    }

    void accept(TypeVisitor &visitor) const override {
        visitor.visitI1(*this);
    }

protected:
    bool equals(const Type &other) const override {
        return typeid(*this) == typeid(other);
    }
};

} // namespace mini_llvm::ir
