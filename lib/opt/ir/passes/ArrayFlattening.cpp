#include "mini-llvm/opt/ir/passes/ArrayFlattening.h"

#include <memory>
#include <vector>

#include "mini-llvm/ir/BasicBlock.h"
#include "mini-llvm/ir/Function.h"
#include "mini-llvm/ir/Instruction.h"
#include "mini-llvm/ir/Instruction/Add.h"
#include "mini-llvm/ir/Instruction/GetElementPtr.h"
#include "mini-llvm/ir/Instruction/Mul.h"
#include "mini-llvm/ir/Type.h"
#include "mini-llvm/ir/Type/ArrayType.h"
#include "mini-llvm/ir/Type/I8.h"
#include "mini-llvm/ir/Type/Ptr.h"
#include "mini-llvm/ir/Use.h"
#include "mini-llvm/ir/Value.h"

using namespace mini_llvm::ir;

bool ArrayFlattening::runOnFunction(Function &F) {
    bool changed = false;

    for (BasicBlock &B : F) {
        for (Instruction &I : B) {
            if (auto *gep = dynamic_cast<GetElementPtr *>(&I)) {
                std::unique_ptr<Type> type = gep->sourceType();
                while (dynamic_cast<const ArrayType *>(&*type)) {
                    type = static_cast<const ArrayType *>(&*type)->elementType();
                }
                size_t n = gep->idx_size();
                if (!(*type == I8() && n == 1) && *type != Ptr()) {
                    std::vector<size_t> sizes;
                    type = gep->sourceType();
                    for (;;) {
                        sizes.push_back(type->size());
                        if (!dynamic_cast<const ArrayType *>(&*type)) {
                            break;
                        }
                        type = static_cast<const ArrayType *>(&*type)->elementType();
                    }
                    std::vector<std::shared_ptr<Value>> terms;
                    for (size_t i = 0; i < n; ++i) {
                        std::shared_ptr<Instruction> mul = std::make_shared<Mul>(
                            share(*gep->idx_begin()[i]), gep->idx_begin()[i]->type()->constant(sizes[i])
                        );
                        addToParent(*gep, mul);
                        terms.push_back(mul);
                    }
                    std::shared_ptr<Value> sum = terms[0];
                    for (size_t i = 1; i < n; ++i) {
                        std::shared_ptr<Instruction> add = std::make_shared<Add>(sum, terms[i]);
                        addToParent(*gep, add);
                        sum = add;
                    }
                    std::vector<std::shared_ptr<Value>> indices;
                    indices.push_back(sum);
                    std::shared_ptr<Instruction> newGep = std::make_shared<GetElementPtr>(
                        std::make_unique<I8>(), share(*gep->ptr()), std::move(indices)
                    );
                    addToParent(*gep, newGep);
                    replaceAllUsesWith(*gep, newGep);
                    changed = true;
                }
            }
        }
    }

    return changed;
}
