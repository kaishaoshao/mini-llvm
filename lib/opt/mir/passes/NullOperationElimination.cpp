#include "mini-llvm/opt/mir/passes/NullOperationElimination.h"

#include <vector>

#include "mini-llvm/mir/BasicBlock.h"
#include "mini-llvm/mir/Instruction/AddI.h"
#include "mini-llvm/mir/Instruction/AndI.h"
#include "mini-llvm/mir/Instruction/FMov.h"
#include "mini-llvm/mir/Instruction/Mov.h"
#include "mini-llvm/mir/Instruction/OrI.h"
#include "mini-llvm/mir/Instruction/SHLI.h"
#include "mini-llvm/mir/Instruction/SHRAI.h"
#include "mini-llvm/mir/Instruction/SHRLI.h"
#include "mini-llvm/mir/Instruction/XorI.h"
#include "mini-llvm/mir/IntegerImmediate.h"
#include "mini-llvm/mir/RegisterOperand.h"

using namespace mini_llvm::mir;

bool NullOperationElimination::runOnBasicBlock(BasicBlock &B) {
    bool changed = false;

    std::vector<BasicBlock::const_iterator> remove;

    for (auto i = B.begin(), e = B.end(); i != e; ++i) {
        if (auto *mov = dynamic_cast<const Mov *>(&*i)) {
            if (mov->width() == regWidth_) {
                if (&*mov->dst() == &*mov->src()) {
                    remove.emplace_back(i);
                }
            }
        }
        if (auto *fmov = dynamic_cast<const FMov *>(&*i)) {
            if (&*fmov->dst() == &*fmov->src()) {
                remove.emplace_back(i);
            }
        }
        if (auto *addi = dynamic_cast<const AddI *>(&*i)) {
            if (addi->width() == regWidth_) {
                if (&*addi->dst() == &*addi->src1()) {
                    if (auto *imm = dynamic_cast<const IntegerImmediate *>(&*addi->src2())) {
                        if (imm->value() == 0) {
                            remove.emplace_back(i);
                        }
                    }
                }
            }
        }
        if (auto *andi = dynamic_cast<const AndI *>(&*i)) {
            if (andi->width() == regWidth_) {
                if (&*andi->dst() == &*andi->src1()) {
                    if (auto *imm = dynamic_cast<const IntegerImmediate *>(&*andi->src2())) {
                        if (imm->value() == -1) {
                            remove.emplace_back(i);
                        }
                    }
                }
            }
        }
        if (auto *ori = dynamic_cast<const OrI *>(&*i)) {
            if (ori->width() == regWidth_) {
                if (&*ori->dst() == &*ori->src1()) {
                    if (auto *imm = dynamic_cast<const IntegerImmediate *>(&*ori->src2())) {
                        if (imm->value() == 0) {
                            remove.emplace_back(i);
                        }
                    }
                }
            }
        }
        if (auto *xori = dynamic_cast<const XorI *>(&*i)) {
            if (xori->width() == regWidth_) {
                if (&*xori->dst() == &*xori->src1()) {
                    if (auto *imm = dynamic_cast<const IntegerImmediate *>(&*xori->src2())) {
                        if (imm->value() == 0) {
                            remove.emplace_back(i);
                        }
                    }
                }
            }
        }
        if (auto *shli = dynamic_cast<const SHLI *>(&*i)) {
            if (shli->width() == regWidth_) {
                if (&*shli->dst() == &*shli->src1()) {
                    if (auto *imm = dynamic_cast<const IntegerImmediate *>(&*shli->src2())) {
                        if (imm->value() == 0) {
                            remove.emplace_back(i);
                        }
                    }
                }
            }
        }
        if (auto *shrli = dynamic_cast<const SHRLI *>(&*i)) {
            if (shrli->width() == regWidth_) {
                if (&*shrli->dst() == &*shrli->src1()) {
                    if (auto *imm = dynamic_cast<const IntegerImmediate *>(&*shrli->src2())) {
                        if (imm->value() == 0) {
                            remove.emplace_back(i);
                        }
                    }
                }
            }
        }
        if (auto *shrai = dynamic_cast<const SHRAI *>(&*i)) {
            if (shrai->width() == regWidth_) {
                if (&*shrai->dst() == &*shrai->src1()) {
                    if (auto *imm = dynamic_cast<const IntegerImmediate *>(&*shrai->src2())) {
                        if (imm->value() == 0) {
                            remove.emplace_back(i);
                        }
                    }
                }
            }
        }
    }

    for (auto i : remove) {
        B.remove(i);
        changed = true;
    }

    return changed;
}
