// ============================================================================
//  History.h - DSA: DOUBLY LINKED LIST of past games (newest at the head).
// ============================================================================
#pragma once
#include <string>
#include <vector>
#include "Types.h"
#include "ds/LinkedList.h"

class History {
public:
    explicit History(const std::string& path) : path_(path) {}
    bool load();
    bool save() const;
    int  add(GameRecord rec);                                  // assigns id + date, returns id
    int  nextId() const { return nextId_; }
    const LinkedList<GameRecord>& list() const { return list_; }
    std::vector<GameRecord> forPlayer(const std::string& name) const;   // newest first
private:
    std::string path_;
    LinkedList<GameRecord> list_;
    int nextId_ = 1;
};
