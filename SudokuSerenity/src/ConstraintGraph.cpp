#include "ds/Graph.h"
#include "ds/Queue.h"
#include "ds/Stack.h"

SudokuGraph::SudokuGraph(const GridSpec& spec) : spec_(spec) {
    const int N = spec.n * spec.n;
    adj_.assign(static_cast<std::size_t>(N), std::vector<int>());
    for (int a = 0; a < 81; ++a)
        for (int b = 0; b < 81; ++b) matrix_[a][b] = false;

    for (int a = 0; a < N; ++a) {
        int ra = a / spec.n, ca = a % spec.n;
        for (int b = 0; b < N; ++b) {
            if (a == b) continue;
            int rb = b / spec.n, cb = b % spec.n;
            bool sameRow = (ra == rb);
            bool sameCol = (ca == cb);
            bool sameBox = (ra / spec.boxR == rb / spec.boxR) && (ca / spec.boxC == cb / spec.boxC);
            if (sameRow || sameCol || sameBox) {
                matrix_[a][b] = true;
                adj_[static_cast<std::size_t>(a)].push_back(b);
            }
        }
    }
    int sum = 0;
    for (int v = 0; v < N; ++v) sum += static_cast<int>(adj_[static_cast<std::size_t>(v)].size());
    edges_ = sum / 2;
}

const SudokuGraph& SudokuGraph::forSize(int n) {
    static const SudokuGraph g4(GridSpec{4, 2, 2});
    static const SudokuGraph g6(GridSpec{6, 2, 3});
    static const SudokuGraph g9(GridSpec{9, 3, 3});
    return n == 4 ? g4 : (n == 6 ? g6 : g9);
}

std::vector<int> SudokuGraph::bfsWithin(int src, int maxDepth) const {
    std::vector<int> result;
    std::vector<int> dist(static_cast<std::size_t>(vertexCount()), -1);
    Queue<int> frontier;
    dist[static_cast<std::size_t>(src)] = 0;
    frontier.enqueue(src);
    while (!frontier.empty()) {
        int v = frontier.dequeue();
        result.push_back(v);
        if (dist[static_cast<std::size_t>(v)] == maxDepth) continue;
        for (int w : neighbors(v)) {
            if (dist[static_cast<std::size_t>(w)] == -1) {
                dist[static_cast<std::size_t>(w)] = dist[static_cast<std::size_t>(v)] + 1;
                frontier.enqueue(w);
            }
        }
    }
    return result;
}

bool SudokuGraph::isConnected() const {
    std::vector<bool> seen(static_cast<std::size_t>(vertexCount()), false);
    Stack<int> st;
    st.push(0);
    seen[0] = true;
    int visited = 0;
    while (!st.empty()) {
        int v = st.pop();
        ++visited;
        for (int w : neighbors(v)) {
            if (!seen[static_cast<std::size_t>(w)]) {
                seen[static_cast<std::size_t>(w)] = true;
                st.push(w);
            }
        }
    }
    return visited == vertexCount();
}

bool SudokuGraph::hasConflict(const int (*grid)[9], int r, int c) const {
    int v = grid[r][c];
    if (v == 0) return false;
    for (int w : neighbors(index(r, c)))
        if (grid[w / spec_.n][w % spec_.n] == v) return true;
    return false;
}

std::vector<int> SudokuGraph::conflictingNeighbors(const int (*grid)[9], int r, int c) const {
    std::vector<int> out;
    int val = grid[r][c];
    if (val == 0) return out;
    for (int w : neighbors(index(r, c)))
        if (grid[w / spec_.n][w % spec_.n] == val) out.push_back(w);
    return out;
}

unsigned SudokuGraph::candidateMask(const int (*grid)[9], int r, int c) const {
    unsigned mask = ((1u << (spec_.n + 1)) - 1u) & ~1u;        // digits 1..n
    for (int w : neighbors(index(r, c))) {
        int d = grid[w / spec_.n][w % spec_.n];
        if (d) mask &= ~(1u << d);
    }
    return mask;
}
