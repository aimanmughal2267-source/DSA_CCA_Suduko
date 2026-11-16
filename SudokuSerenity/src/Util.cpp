#include "Util.h"
#include <cctype>
#include <ctime>
#include <filesystem>
#include <sstream>

namespace util {

static std::tm localNow() {
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    std::tm out{};
    if (lt) out = *lt;
    return out;
}

int todayNumber() {
    std::tm t = localNow();
    return (t.tm_year + 1900) * 10000 + (t.tm_mon + 1) * 100 + t.tm_mday;
}

std::string nowString() {
    std::tm t = localNow();
    char buf[32] = "unknown";
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &t);
    return buf;
}

std::string greeting() {
    int h = localNow().tm_hour;
    if (h < 12) return "Good morning";
    if (h < 18) return "Good afternoon";
    return "Good evening";
}

std::string lower(const std::string& s) {
    std::string out;
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}

std::string cleanName(const std::string& s) {
    std::string out;
    for (unsigned char c : s)
        if (std::isalnum(c) || c == ' ' || c == '_' || c == '-' || c == '.') out.push_back(static_cast<char>(c));
    std::size_t a = out.find_first_not_of(' ');
    if (a == std::string::npos) return "";
    std::size_t b = out.find_last_not_of(' ');
    out = out.substr(a, b - a + 1);
    if (out.size() > 14) out.resize(14);
    return out;
}

std::vector<std::string> split(const std::string& line, char sep) {
    std::vector<std::string> out;
    std::string cur;
    std::stringstream ss(line);
    while (std::getline(ss, cur, sep)) out.push_back(cur);
    if (!line.empty() && line.back() == sep) out.push_back("");
    return out;
}

int toInt(const std::string& s) {
    try { return std::stoi(s); } catch (...) { return 0; }
}

std::string dataDir() {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::is_directory("data", ec))    return "data/";
    if (fs::is_directory("../data", ec)) return "../data/";     // running from build/
    fs::create_directories("data", ec);
    return "data/";
}

std::string fileSafe(const std::string& name) {
    std::string out;
    for (unsigned char c : lower(name)) out.push_back(std::isalnum(c) ? static_cast<char>(c) : '_');
    return out;
}

std::string formatTime(int seconds) {
    if (seconds < 0) seconds = 0;
    int m = seconds / 60, s = seconds % 60;
    return (m < 10 ? "0" : "") + std::to_string(m) + ":" + (s < 10 ? "0" : "") + std::to_string(s);
}

} // namespace util
