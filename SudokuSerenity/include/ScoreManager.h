// ============================================================================
//  ScoreManager.h - scoring rules + DSA: BINARY SEARCH TREE of all scores.
//  The BST keeps every completed game ordered, giving O(height) rank queries.
//  A chronological vector is kept only for saving (so reloading does not build
//  a degenerate tree from sorted input).
// ============================================================================
#pragma once
#include <string>
#include <vector>
#include "Types.h"
#include "ds/BST.h"

namespace Scoring {
int baseScore(Difficulty d);
int parSeconds(Difficulty d);
// Final score: base + time bonus - penalties (no time bonus in relaxation mode).
int compute(Difficulty d, int seconds, int mistakes, int hints, bool relaxed, bool daily);
}

class ScoreManager {
public:
    explicit ScoreManager(const std::string& path) : path_(path) {}
    bool load();
    bool save() const;
    void add(const ScoreEntry& e);                       // inserts into the BST and saves
    int  rankOf(const ScoreEntry& e) const;              // 1 = best of all games
    std::size_t count() const { return tree_.size(); }
    int  height() const { return tree_.height(); }
    const ScoreEntry* best() const { return tree_.maximum(); }
    std::vector<ScoreEntry> all() const;                 // in-order traversal (ascending)
    std::vector<ScoreEntry> filtered(int sizeIndex) const;   // -1 = every size

private:
    std::string path_;
    BST<ScoreEntry, ScoreOrder> tree_;
    std::vector<ScoreEntry> chronological_;
};
