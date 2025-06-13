#include "mini-llvm/opt/mir/passes/BranchPredictionAnalysis.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "mini-llvm/mir/BasicBlock.h"
#include "mini-llvm/mir/Function.h"
#include "mini-llvm/utils/Hash.h"

using namespace mini_llvm;
using namespace mini_llvm::mir;

namespace {

using Edge = std::pair<const BasicBlock *, const BasicBlock *>;

enum class Color {
    kWhite,
    kGray,
    kBlack,
};

void dfs(const BasicBlock *u,
         std::unordered_map<const BasicBlock *, Color> &colors,
         std::unordered_set<Edge, Hash<Edge>> &backEdges) {
    colors[u] = Color::kGray;
    for (const BasicBlock *v : successors(*u)) {
        if (colors[v] == Color::kWhite) {
            dfs(v, colors, backEdges);
        } else if (colors[v] == Color::kGray) {
            backEdges.emplace(u, v);
        }
    }
    colors[u] = Color::kBlack;
}

} // namespace

class BranchPredictionAnalysis::Impl {
public:
    void runOnFunction(const Function &F) {
        std::unordered_map<const BasicBlock *, Color> colors;
        std::unordered_set<Edge, Hash<Edge>> backEdges;

        for (const BasicBlock &B : F) {
            colors.emplace(&B, Color::kWhite);
        }

        for (const BasicBlock &B : F) {
            if (colors[&B] == Color::kWhite) {
                dfs(&B, colors, backEdges);
            }
        }

        for (const BasicBlock &B : F) {
            std::unordered_set<const BasicBlock *> predictions;

            for (const BasicBlock *succ : successors(B)) {
                if (backEdges.contains(Edge(&B, succ))) {
                    predictions.insert(succ);
                }
            }

            if (predictions.empty()) {
                for (const BasicBlock *succ : successors(B)) {
                    predictions.insert(succ);
                }
            }

            for (const BasicBlock *prediction : predictions) {
                predictions_.emplace(&B, prediction);
            }
        }
    }

    bool predict(const BasicBlock &B, const BasicBlock &succ) const {
        return predictions_.contains(Edge(&B, &succ));
    }

private:
    std::unordered_set<Edge, Hash<Edge>> predictions_;
};

BranchPredictionAnalysis::BranchPredictionAnalysis() : impl_(std::make_unique<Impl>()) {}

BranchPredictionAnalysis::~BranchPredictionAnalysis() = default;

void BranchPredictionAnalysis::runOnFunction(const Function &F) {
    impl_->runOnFunction(F);
}

bool BranchPredictionAnalysis::predict(const BasicBlock &B, const BasicBlock &succ) const {
    return impl_->predict(B, succ);
}
