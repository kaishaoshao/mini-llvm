#include "mini-llvm/ir/Value.h"

#include <cassert>
#include <memory>

#include "mini-llvm/ir/Use.h"

using namespace mini_llvm;
using namespace mini_llvm::ir;

bool ir::replaceAllUsesWith(const Value &value, std::shared_ptr<Value> newValue) {
    assert(*newValue->type() == *value.type());

    bool changed = false;

    while (!value.use_empty()) {
        value.use_begin()->set(newValue);
        changed = true;
    }

    return changed;
}

bool ir::replaceAllUsesWith(const Value &value, std::weak_ptr<Value> newValue) {
    assert(!newValue.expired() && *newValue.lock()->type() == *value.type());

    bool changed = false;

    while (!value.use_empty()) {
        value.use_begin()->set(newValue);
        changed = true;
    }

    return changed;
}
