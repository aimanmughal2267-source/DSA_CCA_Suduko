// ============================================================================
//  Leaderboard.h - DSA: MAX-HEAP ranking + searching.
//  build(): heapify all score entries, extract best-first, keep one row per
//  player (hash set). Registered players with no wins are appended afterwards.
// ============================================================================
#pragma once
#include <string>
#include <vector>
#include "Player.h"
#include "ScoreManager.h"

struct Ranking {
    std::string name;
    int avatar = 0;
    int bestScore = 0;
    int bestSeconds = 0;
    Difficulty difficulty = Difficulty::Beginner;
    int wins = 0;
    bool ranked = false;                  // false = registered but no completed games yet
};

namespace Leaderboard {
std::vector<Ranking> build(const ScoreManager& scores, const PlayerManager& players, int sizeIndex);  // -1 = all sizes
int positionOf(const std::vector<Ranking>& rows, const std::string& name);        // linear search, -1 if absent
}
