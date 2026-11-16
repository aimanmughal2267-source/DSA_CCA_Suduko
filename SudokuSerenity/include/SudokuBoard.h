// ============================================================================
//  SudokuBoard.h - DSA: TWO-DIMENSIONAL ARRAY holding a 4x4, 6x6 or 9x9 grid
//  (stored in a fixed 9x9 array; only the first n rows/columns are used).
//  Also keeps "given" flags and pencil notes (one bit mask per cell).
// ============================================================================
#pragma once
#include <cstdint>
#include <vector>
#include "Types.h"

class SudokuBoard {
public:
    SudokuBoard() { reset(GridSpec{9, 3, 3}); }
    void reset(const GridSpec& spec);                       // clears everything
    void loadPuzzle(const int (*puzzle)[9]);                // digits become locked givens

    const GridSpec& spec() const { return spec_; }
    int  n() const { return spec_.n; }
    int  get(int r, int c) const { return cells_[r][c]; }
    void set(int r, int c, int v) { cells_[r][c] = v; }
    bool isGiven(int r, int c) const { return given_[r][c]; }

    std::uint16_t notes(int r, int c) const { return notes_[r][c]; }
    void setNotes(int r, int c, std::uint16_t m) { notes_[r][c] = m; }

    int  filledCount() const;
    bool matches(const int (*other)[9]) const;
    void copyTo(int (*out)[9]) const;
    const int (*grid() const)[9] { return cells_; }

    bool hasConflict(int r, int c) const;                   // graph based
    std::vector<int> conflictingCells(int r, int c) const;

private:
    GridSpec      spec_;
    int           cells_[9][9];
    bool          given_[9][9];
    std::uint16_t notes_[9][9];
};
