#include "ScoreManager.h"
#include <algorithm>
#include <fstream>
#include "Util.h"

namespace Scoring {
int baseScore(Difficulty d) { static const int t[DIFFICULTY_COUNT] = { 400, 800, 1500, 2500 }; return t[static_cast<int>(d)]; }
int parSeconds(Difficulty d) { static const int t[DIFFICULTY_COUNT] = { 90, 300, 900, 1500 }; return t[static_cast<int>(d)]; }

int compute(Difficulty d, int seconds, int mistakes, int hints, bool relaxed, bool daily) {
    int bonus = relaxed ? 0 : std::max(0, parSeconds(d) - seconds);
    double score = baseScore(d) + bonus - 40 * mistakes - 60 * hints;
    if (relaxed) score *= 0.9;
    if (daily)   score *= 1.15;
    return std::max(50, static_cast<int>(score));
}
} // namespace Scoring

bool ScoreManager::load() {
    std::ifstream in(path_);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::vector<std::string> f = util::split(line, '|');
        if (f.size() < 6) continue;
        ScoreEntry e;
        e.id = util::toInt(f[0]);
        e.name = f[1];
        e.difficulty = difficultyFromInt(util::toInt(f[2]));
        e.score = util::toInt(f[3]);
        e.seconds = util::toInt(f[4]);
        e.relaxed = util::toInt(f[5]) != 0;
        tree_.insert(e);
        chronological_.push_back(e);
    }
    return true;
}

bool ScoreManager::save() const {
    std::ofstream out(path_, std::ios::trunc);
    if (!out) return false;
    for (const ScoreEntry& e : chronological_)
        out << e.id << '|' << e.name << '|' << static_cast<int>(e.difficulty) << '|' << e.score << '|'
            << e.seconds << '|' << (e.relaxed ? 1 : 0) << "\n";
    return static_cast<bool>(out);
}

void ScoreManager::add(const ScoreEntry& e) {
    tree_.insert(e);
    chronological_.push_back(e);
    save();
}

int ScoreManager::rankOf(const ScoreEntry& e) const { return static_cast<int>(tree_.countGreater(e)) + 1; }

std::vector<ScoreEntry> ScoreManager::all() const {
    std::vector<ScoreEntry> v;
    tree_.inorder([&](const ScoreEntry& e) { v.push_back(e); return true; });
    return v;
}

std::vector<ScoreEntry> ScoreManager::filtered(int sizeIndex) const {
    std::vector<ScoreEntry> v;
    tree_.inorder([&](const ScoreEntry& e) {
        if (sizeIndex < 0 || sizeIndexOf(e.difficulty) == sizeIndex) v.push_back(e);
        return true;
    });
    return v;
}
