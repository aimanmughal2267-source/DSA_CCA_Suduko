#include "SudokuSolver.h"
#include "ds/Graph.h"

namespace Solver {

namespace {

int popCount(unsigned m) { int k = 0; while (m) { m &= m - 1; ++k; } return k; }

// MRV heuristic: choose the empty cell with the fewest candidates.
// returns 1 = cell found, 0 = dead end, -1 = grid complete
int pickCell(const GridSpec& s, const int (*g)[9], int& br, int& bc, unsigned& bmask) {
    const SudokuGraph& G = SudokuGraph::forSize(s.n);
    int best = 100;
    bool any = false;
    for (int r = 0; r < s.n; ++r)
        for (int c = 0; c < s.n; ++c) {
            if (g[r][c] != 0) continue;
            unsigned m = G.candidateMask(g, r, c);
            int k = popCount(m);
            if (k == 0) return 0;
            if (k < best) { best = k; br = r; bc = c; bmask = m; any = true; if (k == 1) return 1; }
        }
    return any ? 1 : -1;
}

bool solveRec(const GridSpec& s, int (*g)[9]) {
    int r = 0, c = 0; unsigned m = 0;
    int st = pickCell(s, g, r, c, m);
    if (st == -1) return true;
    if (st == 0)  return false;
    for (int d = 1; d <= s.n; ++d) {
        if (!((m >> d) & 1u)) continue;
        g[r][c] = d;
        if (solveRec(s, g)) return true;          // recursion
    }
    g[r][c] = 0;                                  // backtrack
    return false;
}

int countRec(const GridSpec& s, int (*g)[9], int limit) {
    int r = 0, c = 0; unsigned m = 0;
    int st = pickCell(s, g, r, c, m);
    if (st == -1) return 1;
    if (st == 0)  return 0;
    int total = 0;
    for (int d = 1; d <= s.n && total < limit; ++d) {
        if (!((m >> d) & 1u)) continue;
        g[r][c] = d;
        total += countRec(s, g, limit - total);
    }
    g[r][c] = 0;                                  // backtrack
    return total;
}

} // namespace

bool isValidPlacement(const GridSpec& s, const int (*g)[9], int r, int c, int value) {
    if (value < 1 || value > s.n) return false;
    const SudokuGraph& G = SudokuGraph::forSize(s.n);
    for (int w : G.neighbors(G.index(r, c)))
        if (g[w / s.n][w % s.n] == value) return false;
    return true;
}

bool isConsistent(const GridSpec& s, const int (*g)[9]) {
    const SudokuGraph& G = SudokuGraph::forSize(s.n);
    for (int r = 0; r < s.n; ++r)
        for (int c = 0; c < s.n; ++c)
            if (G.hasConflict(g, r, c)) return false;
    return true;
}

bool isSolved(const GridSpec& s, const int (*g)[9]) {
    for (int r = 0; r < s.n; ++r)
        for (int c = 0; c < s.n; ++c)
            if (g[r][c] == 0) return false;
    return isConsistent(s, g);
}

bool solve(const GridSpec& s, int (*g)[9]) { return solveRec(s, g); }
int  countSolutions(const GridSpec& s, int (*g)[9], int limit) { return countRec(s, g, limit); }

std::vector<HintStep> buildHintOrder(const GridSpec& s, const int (*puzzle)[9], const int (*solution)[9]) {
    const SudokuGraph& G = SudokuGraph::forSize(s.n);
    int work[9][9];
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) work[r][c] = puzzle[r][c];
    std::vector<HintStep> order;
    for (;;) {
        int bestR = -1, bestC = -1, bestK = 100;
        for (int r = 0; r < s.n; ++r)
            for (int c = 0; c < s.n; ++c) {
                if (work[r][c] != 0) continue;
                int k = popCount(G.candidateMask(work, r, c));
                if (k < bestK) { bestK = k; bestR = r; bestC = c; }
            }
        if (bestR < 0) break;
        work[bestR][bestC] = solution[bestR][bestC];
        order.push_back({ bestR, bestC, solution[bestR][bestC], bestK });
    }
    return order;
}

} // namespace Solver
