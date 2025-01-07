#include "mini-llvm/targets/riscv/mir/RISCVRegister.h"

#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <utility>

#include "mini-llvm/mir/RegisterClass.h"

using namespace mini_llvm::mir;

RISCVRegister *RISCVRegister::get(int idx) {
    static std::unordered_map<int, std::shared_ptr<RISCVRegister>> pool;

    if (pool.contains(idx)) {
        return &*pool[idx];
    }

    switch (idx) {
#define REGS
#define X(idx, name, class, isPreserved, isAllocatable) \
    case idx: \
        return &*pool.insert( \
            {idx, std::shared_ptr<RISCVRegister>(new RISCVRegister(idx, #name, RegisterClass::k##class, isPreserved, isAllocatable))}).first->second;
#include "mini-llvm/targets/riscv/target.def"
#undef X
#undef REGS
    default:
        abort();
    }
}
