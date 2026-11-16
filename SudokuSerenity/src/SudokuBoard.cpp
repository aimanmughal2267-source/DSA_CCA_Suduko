#include "SudokuBoard.h"
#include "ds/Graph.h"

void SudokuBoard::reset(const GridSpec& spec) {
    spec_ = spec;
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) {
            cells_[r][c] = 0;
            given_[r][c] = false;
            notes_[r][c] = 0;
        }
}

void SudokuBoard::loadPuzzle(const int (*puzzle)[9]) {
    for (int r = 0; r < spec_.n; ++r)
        for (int c = 0; c < spec_.n; ++c) {
            cells_[r][c] = puzzle[r][c];
            given_[r][c] = (puzzle[r][c] != 0);
            notes_[r][c] = 0;
        }
}

int SudokuBoard::filledCount() const {
    int k = 0;
    for (int r = 0; r < spec_.n; ++r)
        for (int c = 0; c < spec_.n; ++c)
            if (cells_[r][c] != 0) ++k;
    return k;
}

bool SudokuBoard::matches(const int (*other)[9]) const {
    for (int r = 0; r < spec_.n; ++r)
        for (int c = 0; c < spec_.n; ++c)
            if (cells_[r][c] != other[r][c]) return false;
    return true;
}

void SudokuBoard::copyTo(int (*out)[9]) const {
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) out[r][c] = cells_[r][c];
}

bool SudokuBoard::hasConflict(int r, int c) const {
    return SudokuGraph::forSize(spec_.n).hasConflict(cells_, r, c);
}

std::vector<int> SudokuBoard::conflictingCells(int r, int c) const {
    return SudokuGraph::forSize(spec_.n).conflictingNeighbors(cells_, r, c);
}
