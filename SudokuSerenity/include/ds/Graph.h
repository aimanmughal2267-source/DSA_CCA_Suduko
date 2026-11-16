// ============================================================================
//  Graph.h - DSA: GRAPH (undirected; adjacency list + adjacency matrix).
//  Every Sudoku cell is a vertex. Two cells are joined by an edge when they
//  share a row, a column or a sub-grid (box). One graph exists per grid size:
//     4x4 -> 16 vertices, degree 7      6x6 -> 36 vertices, degree 12
//     9x9 -> 81 vertices, degree 20
//  Algorithms: BFS (Queue), DFS (Stack), conflict detection, candidate colours.
// ============================================================================
#pragma once
#include <vector>
#include "Types.h"

class SudokuGraph {
public:
    explicit SudokuGraph(const GridSpec& spec);
    static const SudokuGraph& forSize(int n);            // cached graph for 4, 6 or 9

    int  n() const { return spec_.n; }
    int  vertexCount() const { return spec_.n * spec_.n; }
    int  index(int r, int c) const { return r * spec_.n + c; }
    const std::vector<int>& neighbors(int v) const { return adj_[static_cast<std::size_t>(v)]; }
    bool areAdjacent(int a, int b) const { return matrix_[a][b]; }
    int  degree(int v) const { return static_cast<int>(neighbors(v).size()); }
    int  edgeCount() const { return edges_; }

    std::vector<int> bfsWithin(int src, int maxDepth) const;   // BFS using a Queue
    bool isConnected() const;                                   // DFS using a Stack

    bool hasConflict(const int (*grid)[9], int r, int c) const;
    std::vector<int> conflictingNeighbors(const int (*grid)[9], int r, int c) const;
    unsigned candidateMask(const int (*grid)[9], int r, int c) const;   // bit d set => digit d allowed

private:
    GridSpec spec_;
    std::vector<std::vector<int>> adj_;
    bool matrix_[81][81];
    int  edges_ = 0;
};
