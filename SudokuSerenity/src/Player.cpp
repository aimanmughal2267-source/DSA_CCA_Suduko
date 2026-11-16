#include "Player.h"
#include <algorithm>
#include <fstream>
#include "Achievements.h"
#include "Algorithms.h"
#include "Util.h"

int PlayerProfile::playedTotal() const { int s = 0; for (int v : played) s += v; return s; }
int PlayerProfile::completedTotal() const { int s = 0; for (int v : completed) s += v; return s; }
int PlayerProfile::completedForSize(int si) const {
    int s = 0;
    for (int d = 0; d < DIFFICULTY_COUNT; ++d)
        if (sizeIndexOf(difficultyFromInt(d)) == si) s += completed[d];
    return s;
}
int PlayerProfile::bestTimeForSize(int si) const {
    int best = 0;
    for (int d = 0; d < DIFFICULTY_COUNT; ++d)
        if (sizeIndexOf(difficultyFromInt(d)) == si && bestTime[d] > 0 && (best == 0 || bestTime[d] < best)) best = bestTime[d];
    return best;
}
int PlayerProfile::highestScore() const { int m = 0; for (int v : bestScore) m = std::max(m, v); return m; }
double PlayerProfile::averageMistakes() const {
    int c = completedTotal();
    return c ? static_cast<double>(totalMistakes) / c : 0.0;
}

// ------------------------------ persistence ----------------------------------
bool PlayerManager::load() {
    std::ifstream in(path_);
    if (!in) return false;
    std::string line, last;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::vector<std::string> f = util::split(line, '|');
        if (f.empty()) continue;
        if (f[0] == "LAST" && f.size() >= 2) { last = f[1]; continue; }
        if (f[0] != "PLAYER" || f.size() < 24) continue;
        PlayerProfile p;
        p.name = f[1];
        p.avatar = util::toInt(f[2]);
        for (int i = 0; i < DIFFICULTY_COUNT; ++i) {
            p.played[i]    = util::toInt(f[3 + i]);
            p.completed[i] = util::toInt(f[7 + i]);
            p.bestTime[i]  = util::toInt(f[11 + i]);
            p.bestScore[i] = util::toInt(f[15 + i]);
        }
        p.totalMistakes = util::toInt(f[19]);
        p.totalHints    = util::toInt(f[20]);
        p.totalScore    = util::toInt(f[21]);
        p.achievements  = static_cast<unsigned>(util::toInt(f[22]));
        p.dailyLast     = util::toInt(f[23]);
        table_.insert(util::lower(p.name), p);
    }
    if (!last.empty() && table_.contains(util::lower(last))) currentKey_ = util::lower(last);
    return true;
}

bool PlayerManager::save() const {
    std::ofstream out(path_, std::ios::trunc);
    if (!out) return false;
    const PlayerProfile* cur = currentKey_.empty() ? nullptr : table_.find(currentKey_);
    out << "LAST|" << (cur ? cur->name : std::string()) << "\n";
    table_.forEach([&](const std::string&, const PlayerProfile& p) {
        out << "PLAYER|" << p.name << '|' << p.avatar;
        for (int v : p.played) out << '|' << v;
        for (int v : p.completed) out << '|' << v;
        for (int v : p.bestTime) out << '|' << v;
        for (int v : p.bestScore) out << '|' << v;
        out << '|' << p.totalMistakes << '|' << p.totalHints << '|' << p.totalScore << '|'
            << p.achievements << '|' << p.dailyLast << "\n";
    });
    return static_cast<bool>(out);
}

// ------------------------------- profiles ------------------------------------
PlayerProfile* PlayerManager::add(const std::string& name, int avatar) {
    std::string clean = util::cleanName(name);
    if (clean.empty()) return nullptr;
    std::string key = util::lower(clean);
    if (table_.contains(key)) return nullptr;
    PlayerProfile p;
    p.name = clean;
    p.avatar = avatar < 0 ? 0 : avatar % 6;
    table_.insert(key, p);
    currentKey_ = key;
    save();
    return table_.find(key);
}

bool PlayerManager::switchTo(const std::string& name) {
    std::string key = util::lower(util::cleanName(name));
    if (!table_.contains(key)) return false;
    currentKey_ = key;
    save();
    return true;
}

PlayerProfile* PlayerManager::current() { return currentKey_.empty() ? nullptr : table_.find(currentKey_); }
PlayerProfile* PlayerManager::find(const std::string& name) { return table_.find(util::lower(name)); }
const PlayerProfile* PlayerManager::find(const std::string& name) const { return table_.find(util::lower(name)); }

std::vector<std::string> PlayerManager::names() const {
    std::vector<std::string> v;
    table_.forEach([&](const std::string&, const PlayerProfile& p) { v.push_back(p.name); });
    Algo::quickSort(v, [](const std::string& a, const std::string& b) { return util::lower(a) < util::lower(b); });
    return v;
}

std::string PlayerManager::findExact(const std::string& query) const {
    std::vector<std::string> v = names();
    std::string key = util::lower(util::cleanName(query));
    if (key.empty()) return "";
    int idx = Algo::binarySearch(v, key, [](const std::string& e, const std::string& k) { return util::lower(e).compare(k); });
    return idx >= 0 ? v[static_cast<std::size_t>(idx)] : std::string();
}

void PlayerManager::recordStart(Difficulty d) {
    if (PlayerProfile* p = current()) { ++p->played[static_cast<int>(d)]; save(); }
}

PersonalBest PlayerManager::recordFinish(const GameRecord& rec, unsigned& newlyUnlocked) {
    PersonalBest pb;
    newlyUnlocked = 0;
    PlayerProfile* p = find(rec.player);
    if (!p || !rec.completed) return pb;
    int d = static_cast<int>(rec.difficulty);
    ++p->completed[d];
    p->totalMistakes += rec.mistakes;
    p->totalHints += rec.hints;
    p->totalScore += rec.score;
    if (rec.score > p->bestScore[d]) { if (p->bestScore[d] > 0) pb.score = true; p->bestScore[d] = rec.score; }
    if (!rec.relaxed && (p->bestTime[d] == 0 || rec.seconds < p->bestTime[d])) {
        if (p->bestTime[d] > 0) pb.time = true;
        p->bestTime[d] = rec.seconds;
    }
    if (rec.daily) p->dailyLast = util::todayNumber();
    unsigned before = p->achievements;
    p->achievements = evaluateAchievements(*p, rec);
    newlyUnlocked = p->achievements & ~before;
    save();
    return pb;
}
