// ============================================================================
//  Player.h - player profiles.
//  DSA: HASH TABLE (name -> profile, O(1) average lookup, chaining + rehashing)
//       quick sort for the alphabetical list, binary search by name.
// ============================================================================
#pragma once
#include <string>
#include <vector>
#include "Types.h"
#include "ds/HashTable.h"

struct PlayerProfile {
    std::string name;
    int avatar = 0;
    int played[DIFFICULTY_COUNT]    = {0, 0, 0, 0};
    int completed[DIFFICULTY_COUNT] = {0, 0, 0, 0};
    int bestTime[DIFFICULTY_COUNT]  = {0, 0, 0, 0};      // 0 = none yet (timed games only)
    int bestScore[DIFFICULTY_COUNT] = {0, 0, 0, 0};
    int totalMistakes = 0;                               // over completed games
    int totalHints = 0;
    int totalScore = 0;
    unsigned achievements = 0;
    int dailyLast = 0;                                   // yyyymmdd of last completed daily challenge

    int  playedTotal() const;
    int  completedTotal() const;
    int  completedForSize(int sizeIndex) const;
    int  bestTimeForSize(int sizeIndex) const;           // 0 = none
    int  highestScore() const;
    double averageMistakes() const;
};

struct PersonalBest { bool time = false; bool score = false; };

class PlayerManager {
public:
    explicit PlayerManager(const std::string& path) : path_(path) {}
    bool load();
    bool save() const;

    PlayerProfile* add(const std::string& name, int avatar);   // nullptr if invalid or duplicate
    bool switchTo(const std::string& name);
    PlayerProfile* current();
    const PlayerProfile* current() const { return currentKey_.empty() ? nullptr : table_.find(currentKey_); }
    PlayerProfile* find(const std::string& name);
    const PlayerProfile* find(const std::string& name) const;
    std::vector<std::string> names() const;                    // alphabetical (quick sort)
    std::string findExact(const std::string& query) const;     // binary search, "" if missing
    std::size_t count() const { return table_.size(); }
    std::size_t buckets() const { return table_.bucketCount(); }
    double loadFactor() const { return table_.loadFactor(); }

    void recordStart(Difficulty d);
    // Updates counters/achievements. Returns newly unlocked achievements via mask.
    PersonalBest recordFinish(const GameRecord& rec, unsigned& newlyUnlocked);

private:
    std::string path_;
    HashTable<PlayerProfile> table_;
    std::string currentKey_;
};
