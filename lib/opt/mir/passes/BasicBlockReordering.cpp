#include "mini-llvm/opt/mir/passes/BasicBlockReordering.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mini-llvm/mir/BasicBlock.h"
#include "mini-llvm/mir/Function.h"
#include "mini-llvm/opt/mir/passes/BranchPredictionAnalysis.h"

using namespace mini_llvm::mir;

namespace {

// Held-Karp algorithm

std::vector<int> dp(int n, const std::vector<std::vector<double>> &D, int s) {
    assert(n > 2);
    std::vector<std::vector<double>> f(1 << n, std::vector<double>(n));
    for (int v = 0; v < n; ++v) {
        if (v != s) {
            f[0][v] = D[s][v];
        }
    }
    for (int S = 1; S < (1 << n); ++S) {
        for (int v = 0; v < n; ++v) {
            if (v != s && !((S >> s) & 1) && !((S >> v) & 1)) {
                f[S][v] = std::numeric_limits<double>::infinity();
                for (int u = 0; u < n; ++u) {
                    if ((S >> u) & 1) {
                        f[S][v] = std::min(f[S][v], f[S & ~(1 << u)][u] + D[u][v]);
                    }
                }
            }
        }
    }
    int t;
    double min = std::numeric_limits<double>::infinity();
    for (int v = 0; v < n; ++v) {
        if (v != s && f[((1 << n) - 1) & ~(1 << s) & ~(1 << v)][v] < min) {
            t = v;
            min = f[((1 << n) - 1) & ~(1 << s) & ~(1 << v)][v];
        }
    }
    int S = ((1 << n) - 1) & ~(1 << s) & ~(1 << t);
    std::vector<int> path(n);
    path[0] = t;
    for (int i = 1; i < n - 1; ++i) {
        int v;
        double min = std::numeric_limits<double>::infinity();
        for (int u = 0; u < n; ++u) {
            if (u != t && ((S >> u) & 1) && f[S & ~(1 << u)][u] + D[u][t] < min) {
                v = u;
                min = f[S & ~(1 << u)][u] + D[u][t];
            }
        }
        t = v;
        S &= ~(1 << v);
        path[i] = t;
    }
    path[n - 1] = s;
    std::reverse(path.begin(), path.end());
    return path;
}

// Ant Colony Optimization (ACO)

template <typename RNG>
std::vector<int> aco(
    int n,
    const std::vector<std::vector<double>> &D,
    int s,
    int m,
    double alpha,
    double beta,
    double rho,
    double Q,
    const std::vector<int> &initialPath,
    int maxIter,
    RNG &&rng
) {
    std::vector<std::vector<double>> tau(n, std::vector<double>(n, 1.));
    std::vector<int> bestPath = initialPath;
    double bestPathLength = 0.;
    for (int i = 1; i < n; ++i) {
        bestPathLength += D[initialPath[i - 1]][initialPath[i]];
    }
    for (int iter = 0; iter < maxIter; ++iter) {
        std::vector<std::vector<int>> paths;
        for (int i = 0; i < m; ++i) {
            std::vector<int> path(n);
            path[0] = s;
            for (int j = 1; j < n; ++j) {
                std::vector<double> weights(n);
                for (int k = 0; k < n; ++k) {
                    weights[k] = pow(tau[path[j - 1]][k], alpha) * pow(1. / (D[path[j - 1]][k] + 1e-10), beta);
                }
                for (int k = 0; k < j; ++k) {
                    weights[path[k]] = 0.;
                }
                path[j] = std::discrete_distribution<int>(weights.begin(), weights.end())(rng);
            }
            paths.push_back(path);
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                tau[i][j] *= 1. - rho;
            }
        }
        for (int i = 0; i < m; ++i) {
            double pathLength = 0.;
            for (int j = 1; j < n; ++j) {
                pathLength += D[paths[i][j - 1]][paths[i][j]];
            }
            for (int j = 1; j < n; ++j) {
                tau[paths[i][j - 1]][paths[i][j]] += Q / (pathLength + 1e-10);
            }
            if (pathLength < bestPathLength) {
                bestPath = paths[i];
                bestPathLength = pathLength;
            }
        }
    }
    return bestPath;
}

} // namespace

bool BasicBlockReordering::runOnFunction(Function &F) {
    int n = F.size();

    if (n <= 2) return false;

    BranchPredictionAnalysis predictor;
    predictor.runOnFunction(F);

    std::unordered_map<const BasicBlock *, int> indices;
    for (auto [i, B] : std::views::enumerate(F)) {
        indices.emplace(&B, i);
    }

    std::vector<std::vector<double>> D(n, std::vector<double>(n, 1e+10));
    for (const BasicBlock &B : F) {
        for (const BasicBlock *succ : successors(B)) {
            D[indices[&B]][indices[succ]] = static_cast<double>(!predictor.predict(B, *succ));
        }
    }

    std::vector<int> initialPath(n);
    for (int i = 0; i < n; ++i) {
        initialPath[i] = i;
    }

    std::vector<int> bestPath;

    if (n <= 16) {
        bestPath = dp(n, D, 0);
    } else {
        std::mt19937 rng(42);
        bestPath = aco(n, D, 0, 10, 1., 1., 0.1, 1., initialPath, 100, rng);
    }

    if (bestPath == initialPath) return false;

    std::vector<std::unique_ptr<BasicBlock>> tmp;
    while (!F.empty()) {
        tmp.push_back(F.removeFirst());
    }
    for (int i : bestPath) {
        F.append(std::move(tmp[i]));
    }

    return true;
}
