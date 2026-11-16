// ============================================================================
//  Types.h - shared plain data types: grid specification, difficulty, records.
// ============================================================================
#pragma once
#include <string>

// A grid is described by its size and the shape of its sub-grids (boxes).
//   4x4 : boxes 2x2     6x6 : boxes 2 rows x 3 cols     9x9 : boxes 3x3
struct GridSpec {
    int n    = 9;
    int boxR = 3;
    int boxC = 3;
};
inline constexpr int MAX_N = 9;

enum class Difficulty { Beginner = 0, Focused = 1, Advanced = 2, Master = 3 };
inline constexpr int DIFFICULTY_COUNT = 4;
inline constexpr int SIZE_COUNT = 3;                       // 4x4, 6x6, 9x9

inline Difficulty difficultyFromInt(int v) {
    if (v < 0) v = 0;
    if (v >= DIFFICULTY_COUNT) v = DIFFICULTY_COUNT - 1;
    return static_cast<Difficulty>(v);
}
inline GridSpec specFor(Difficulty d) {
    switch (d) {
        case Difficulty::Beginner: return GridSpec{4, 2, 2};
        case Difficulty::Focused:  return GridSpec{6, 2, 3};
        default:                   return GridSpec{9, 3, 3};
    }
}
inline int sizeIndexOfN(int n) { return n == 4 ? 0 : (n == 6 ? 1 : 2); }
inline int sizeIndexOf(Difficulty d) { return sizeIndexOfN(specFor(d).n); }
inline const char* sizeLabel(int sizeIndex) {
    static const char* t[SIZE_COUNT] = { "4 x 4", "6 x 6", "9 x 9" };
    return t[sizeIndex];
}
inline const char* difficultyName(Difficulty d) {
    static const char* t[DIFFICULTY_COUNT] = { "Beginner", "Focused", "Advanced", "Master" };
    return t[static_cast<int>(d)];
}
inline const char* levelName(Difficulty d) {
    static const char* t[DIFFICULTY_COUNT] = { "Easy", "Medium", "Hard", "Expert" };
    return t[static_cast<int>(d)];
}
inline int cluesFor(Difficulty d) {                        // target number of given digits
    static const int t[DIFFICULTY_COUNT] = { 8, 16, 32, 24 };
    return t[static_cast<int>(d)];
}
inline int hintLimitFor(Difficulty d) {
    static const int t[DIFFICULTY_COUNT] = { 2, 3, 3, 3 };
    return t[static_cast<int>(d)];
}

// One finished (or given-up) game: a node of the history linked list.
struct GameRecord {
    int         id = 0;
    std::string player;
    Difficulty  difficulty = Difficulty::Beginner;
    bool        completed = false;
    bool        relaxed   = false;
    bool        daily     = false;
    int         seconds   = 0;
    int         mistakes  = 0;
    int         hints     = 0;
    int         score     = 0;
    std::string date;
};

// One leaderboard entry: stored in the BST and extracted through the heap.
struct ScoreEntry {
    std::string name;
    int         score   = 0;
    int         seconds = 0;
    int         id      = 0;
    Difficulty  difficulty = Difficulty::Beginner;
    bool        relaxed = false;
};
// "less" = worse. Higher score wins; ties -> faster time; then earlier game.
struct ScoreOrder {
    bool operator()(const ScoreEntry& a, const ScoreEntry& b) const {
        if (a.score != b.score) return a.score < b.score;
        if (a.seconds != b.seconds) return a.seconds > b.seconds;
        return a.id > b.id;
    }
};
