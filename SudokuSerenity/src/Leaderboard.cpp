#include "Leaderboard.h"
#include "Algorithms.h"
#include "Util.h"
#include "ds/HashTable.h"
#include "ds/MaxHeap.h"

namespace Leaderboard {

std::vector<Ranking> build(const ScoreManager& scores, const PlayerManager& players, int sizeIndex) {
    MaxHeap<ScoreEntry, ScoreOrder> heap;
    heap.build(scores.filtered(sizeIndex));                    // O(n) heapify

    HashTable<bool> seen;
    std::vector<Ranking> rows;
    while (!heap.empty()) {
        ScoreEntry e = heap.pop();                             // O(log n) extract-max
        std::string key = util::lower(e.name);
        if (seen.contains(key)) continue;
        seen.insert(key, true);
        Ranking r;
        r.name = e.name; r.bestScore = e.score; r.bestSeconds = e.seconds; r.difficulty = e.difficulty; r.ranked = true;
        if (const PlayerProfile* p = players.find(e.name)) {
            r.avatar = p->avatar;
            r.wins = sizeIndex < 0 ? p->completedTotal() : p->completedForSize(sizeIndex);
        }
        rows.push_back(r);
    }
    for (const std::string& name : players.names()) {          // alphabetical (quick sort)
        if (seen.contains(util::lower(name))) continue;
        Ranking r;
        r.name = name;
        if (const PlayerProfile* p = players.find(name)) r.avatar = p->avatar;
        rows.push_back(r);
    }
    return rows;
}

int positionOf(const std::vector<Ranking>& rows, const std::string& name) {
    std::string key = util::lower(name);
    return Algo::linearSearch(rows, [&](const Ranking& r) { return util::lower(r.name) == key; });
}

} // namespace Leaderboard
