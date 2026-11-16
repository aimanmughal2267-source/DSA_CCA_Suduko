// ============================================================================
//  PuzzleGenerator.h - ALGORITHMS: puzzle generation for any grid size.
//    1. generateSolved : randomised recursive backtracking fills a full grid
//    2. carve          : removes clues while the solution stays UNIQUE
//  Difficulty decides grid size and the number of clues left.
// ============================================================================
#pragma once
#include <random>
#include "Types.h"

struct Puzzle {
    GridSpec spec;
    int puzzle[9][9];
    int solution[9][9];
};

namespace Generator {
std::mt19937 makeRandomEngine();                               // well-seeded engine
void   generateSolved(const GridSpec& s, int (*g)[9], std::mt19937& rng);
void   carve(const GridSpec& s, const int (*solution)[9], int (*puzzle)[9], int clues, std::mt19937& rng);
Puzzle create(Difficulty d, std::mt19937& rng);
Difficulty dailyDifficulty(int dateNumber);                    // today's grid size + level
Puzzle createDaily(int dateNumber);                            // same puzzle for everyone on a date
} // namespace Generator
