#include "Achievements.h"
#include "Player.h"

static const AchievementDef DEFS[ACH_COUNT] = {
    { "First Bloom",    "Complete your first puzzle." },
    { "Gentle Mind",    "Finish a puzzle with no mistakes." },
    { "Pure Focus",     "Finish a puzzle without any guidance." },
    { "Quiet Moment",   "Finish a puzzle in Relaxation mode." },
    { "Swift Breeze",   "Finish an Advanced puzzle in under 10 minutes." },
    { "Garden Keeper",  "Complete 5 puzzles." },
    { "Three Paths",    "Complete a 4x4, a 6x6 and a 9x9 puzzle." },
    { "Daily Devotion", "Complete a Daily Challenge." },
    { "Sakura Master",  "Complete a Master puzzle." },
    { "Grand Garden",   "Complete 15 puzzles." },
};

const AchievementDef& achievementDef(int index) { return DEFS[index]; }

unsigned evaluateAchievements(const PlayerProfile& p, const GameRecord& rec) {
    unsigned m = p.achievements;
    auto grant = [&](int i, bool cond) { if (cond) m |= (1u << i); };
    int total = p.completedTotal();
    grant(0, total >= 1);
    grant(1, rec.completed && rec.mistakes == 0);
    grant(2, rec.completed && rec.hints == 0);
    grant(3, rec.completed && rec.relaxed);
    grant(4, rec.completed && !rec.relaxed && rec.difficulty == Difficulty::Advanced && rec.seconds < 600);
    grant(5, total >= 5);
    grant(6, p.completedForSize(0) > 0 && p.completedForSize(1) > 0 && p.completedForSize(2) > 0);
    grant(7, rec.completed && rec.daily);
    grant(8, rec.completed && rec.difficulty == Difficulty::Master);
    grant(9, total >= 15);
    return m;
}
