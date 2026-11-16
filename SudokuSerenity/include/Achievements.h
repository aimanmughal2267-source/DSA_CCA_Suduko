// ============================================================================
//  Achievements.h - cherry blossom badges. Stored as a bit mask per player.
// ============================================================================
#pragma once
#include "Types.h"

struct PlayerProfile;

inline constexpr int ACH_COUNT = 10;
struct AchievementDef { const char* name; const char* description; };
const AchievementDef& achievementDef(int index);

// Returns the mask of ALL achievements the profile has earned (profile counters
// must already include `rec`).
unsigned evaluateAchievements(const PlayerProfile& p, const GameRecord& rec);
