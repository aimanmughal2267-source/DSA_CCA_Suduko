// ============================================================================
//  SudokuSolver.h - ALGORITHMS: recursion + backtracking, validation, hints.
//  Every function takes a GridSpec, so ONE implementation serves 4x4, 6x6, 9x9.
// ============================================================================
#pragma once
#include <vector>
#include "Types.h"

namespace Solver {

struct HintStep { int row; int col; int value; int candidates; };

// ---- validation (row, column and sub-grid constraints via the graph) ------
bool isValidPlacement(const GridSpec& s, const int (*g)[9], int r, int c, int value);
bool isConsistent(const GridSpec& s, const int (*g)[9]);            // no duplicates anywhere
bool isSolved(const GridSpec& s, const int (*g)[9]);                 // full and consistent

// ---- recursion + backtracking ----------------------------------------------
bool solve(const GridSpec& s, int (*g)[9]);                          // fills the grid in place
int  countSolutions(const GridSpec& s, int (*g)[9], int limit);      // stops at `limit`

// ---- hint order: simulated solving, easiest cell first -----------------------
std::vector<HintStep> buildHintOrder(const GridSpec& s, const int (*puzzle)[9], const int (*solution)[9]);

} // namespace Solver
