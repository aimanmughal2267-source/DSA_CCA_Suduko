// ============================================================================
//  Util.h - small helpers: dates, text, folders.
// ============================================================================
#pragma once
#include <string>
#include <vector>

namespace util {
int         todayNumber();                 // e.g. 20261108
std::string nowString();                   // "2026-11-08 14:30"
std::string greeting();                    // "Good morning" / "Good afternoon" / "Good evening"
std::string lower(const std::string& s);
std::string cleanName(const std::string& s);        // letters, digits, space, _ - . (max 14), trimmed
std::vector<std::string> split(const std::string& line, char sep);
int         toInt(const std::string& s);
std::string dataDir();                     // folder holding players.txt, scores.txt, history.txt
std::string fileSafe(const std::string& name);       // for save-game file names
std::string formatTime(int seconds);       // mm:ss
} // namespace util
