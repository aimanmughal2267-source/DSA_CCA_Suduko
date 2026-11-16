#include "History.h"
#include <fstream>
#include "Util.h"

bool History::load() {
    std::ifstream in(path_);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {                           // file is oldest-first
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::vector<std::string> f = util::split(line, '|');
        if (f.size() < 11) continue;
        GameRecord r;
        r.id = util::toInt(f[0]);
        r.player = f[1];
        r.difficulty = difficultyFromInt(util::toInt(f[2]));
        r.completed = util::toInt(f[3]) != 0;
        r.relaxed = util::toInt(f[4]) != 0;
        r.daily = util::toInt(f[5]) != 0;
        r.seconds = util::toInt(f[6]);
        r.mistakes = util::toInt(f[7]);
        r.hints = util::toInt(f[8]);
        r.score = util::toInt(f[9]);
        r.date = f[10];
        list_.push_front(r);                                   // newest ends up at the head
        if (r.id >= nextId_) nextId_ = r.id + 1;
    }
    return true;
}

bool History::save() const {
    std::ofstream out(path_, std::ios::trunc);
    if (!out) return false;
    std::vector<const GameRecord*> v;
    for (const GameRecord& r : list_) v.push_back(&r);
    for (std::size_t i = v.size(); i-- > 0;) {                 // write oldest first
        const GameRecord& r = *v[i];
        out << r.id << '|' << r.player << '|' << static_cast<int>(r.difficulty) << '|' << (r.completed ? 1 : 0) << '|'
            << (r.relaxed ? 1 : 0) << '|' << (r.daily ? 1 : 0) << '|' << r.seconds << '|' << r.mistakes << '|'
            << r.hints << '|' << r.score << '|' << r.date << "\n";
    }
    return static_cast<bool>(out);
}

int History::add(GameRecord rec) {
    rec.id = nextId_++;
    rec.date = util::nowString();
    list_.push_front(rec);
    save();
    return rec.id;
}

std::vector<GameRecord> History::forPlayer(const std::string& name) const {
    std::vector<GameRecord> v;
    std::string key = util::lower(name);
    for (const GameRecord& r : list_)
        if (util::lower(r.player) == key) v.push_back(r);
    return v;
}
