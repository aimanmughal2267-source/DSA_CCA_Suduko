#include "PuzzleGenerator.h"
#include <algorithm>
#include <chrono>
#include <numeric>
#include <vector>
#include "SudokuSolver.h"
#include "ds/Graph.h"

namespace Generator {

namespace {
int popCount(unsigned m) { int k = 0; while (m) { m &= m - 1; ++k; } return k; }

// Recursive backtracking with randomised digit order.
bool fillRec(const GridSpec& s, int (*g)[9], std::mt19937& rng) {
    const SudokuGraph& G = SudokuGraph::forSize(s.n);
    int br = -1, bc = -1, best = 100; unsigned bm = 0;
    for (int r = 0; r < s.n; ++r)
        for (int c = 0; c < s.n; ++c) {
            if (g[r][c] != 0) continue;
            unsigned m = G.candidateMask(g, r, c);
            int k = popCount(m);
            if (k == 0) return false;
            if (k < best) { best = k; br = r; bc = c; bm = m; }
        }
    if (br < 0) return true;                                   // grid complete
    std::vector<int> digits;
    for (int d = 1; d <= s.n; ++d)
        if ((bm >> d) & 1u) digits.push_back(d);
    std::shuffle(digits.begin(), digits.end(), rng);
    for (int d : digits) {
        g[br][bc] = d;
        if (fillRec(s, g, rng)) return true;
    }
    g[br][bc] = 0;                                             // backtrack
    return false;
}
} // namespace

std::mt19937 makeRandomEngine() {
    std::random_device rd;
    std::seed_seq seq{ rd(), rd(),
        static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
        static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()) };
    return std::mt19937(seq);
}

void generateSolved(const GridSpec& s, int (*g)[9], std::mt19937& rng) {
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) g[r][c] = 0;
    fillRec(s, g, rng);
}

void carve(const GridSpec& s, const int (*solution)[9], int (*puzzle)[9], int clues, std::mt19937& rng) {
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) puzzle[r][c] = solution[r][c];
    const int total = s.n * s.n;
    std::vector<int> order(static_cast<std::size_t>(total));
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), rng);
    int filled = total;
    for (int idx : order) {
        if (filled <= clues) break;
        int r = idx / s.n, c = idx % s.n;
        int backup = puzzle[r][c];
        puzzle[r][c] = 0;
        if (Solver::countSolutions(s, puzzle, 2) != 1) puzzle[r][c] = backup;   // keep solution unique
        else --filled;
    }
}

Puzzle create(Difficulty d, std::mt19937& rng) {
    Puzzle p;
    p.spec = specFor(d);
    generateSolved(p.spec, p.solution, rng);
    carve(p.spec, p.solution, p.puzzle, cluesFor(d), rng);
    return p;
}

Difficulty dailyDifficulty(int dateNumber) {
    unsigned h = static_cast<unsigned>(dateNumber) * 2654435761u;
    return difficultyFromInt(static_cast<int>((h >> 16) % 4u));
}

Puzzle createDaily(int dateNumber) {
    std::mt19937 rng(static_cast<unsigned>(dateNumber) * 7919u + 17u);
    return create(dailyDifficulty(dateNumber), rng);
}

} // namespace Generator
