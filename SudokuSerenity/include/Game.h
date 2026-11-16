// ============================================================================
//  Game.h - one Sudoku game from start to finish.
//  DSA used here:
//     2D array  -> SudokuBoard (4x4 / 6x6 / 9x9)
//     Stack     -> undo stack and redo stack (a move also remembers the notes
//                  it changed in neighbouring cells)
//     Queue     -> queue of pending guidance (hint) steps
//     Graph     -> conflict detection and note clean-up on neighbours
//     Recursion + backtracking -> puzzle generation and "show solution"
// ============================================================================
#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include "PuzzleGenerator.h"
#include "SudokuBoard.h"
#include "Types.h"
#include "ds/Queue.h"
#include "ds/Stack.h"

enum class GameState { Playing, Completed, GaveUp };
enum class InputResult { Ignored, Placed, NoteToggled, Mistake, Completed };

struct NoteChange { int cell; std::uint16_t previous; };
struct Move {
    int row, col;
    int prevValue, nextValue;
    std::uint16_t prevNotes, nextNotes;
    std::vector<NoteChange> neighbourNotes;      // notes removed from peers by this move
};
struct Hint { int row, col, value, candidates; };

class GameSession {
public:
    GameSession();

    // ---- lifecycle ----
    void newGame(Difficulty d, bool relaxed);
    void newDaily(int dateNumber, bool relaxed);
    bool saveTo(const std::string& path) const;
    bool loadFrom(const std::string& path);      // resumes a saved game
    bool active() const { return active_; }

    // ---- information ----
    const GridSpec& spec() const { return board_.spec(); }
    int  n() const { return board_.n(); }
    Difficulty difficulty() const { return difficulty_; }
    bool relaxed() const { return relaxed_; }
    bool daily() const { return daily_; }
    GameState state() const { return state_; }
    bool finished() const { return state_ != GameState::Playing; }
    bool paused() const { return paused_; }
    const SudokuBoard& board() const { return board_; }
    int  value(int r, int c) const { return board_.get(r, c); }
    std::uint16_t notes(int r, int c) const { return board_.notes(r, c); }
    bool isGiven(int r, int c) const { return board_.isGiven(r, c); }
    bool isHinted(int r, int c) const { return hinted_[r][c]; }
    bool isEditable(int r, int c) const { return !board_.isGiven(r, c) && !hinted_[r][c]; }
    bool isWrong(int r, int c) const { return board_.get(r, c) != 0 && board_.get(r, c) != solution_[r][c]; }
    int  digitsPlaced(int digit) const;          // correct copies of `digit` on the board
    bool rowComplete(int r) const;
    bool colComplete(int c) const;
    bool boxComplete(int r, int c) const;        // the box containing (r, c)

    // ---- actions ----
    InputResult input(int r, int c, int digit);  // writes a number, or toggles a note in notes mode
    bool erase(int r, int c);
    bool undo();
    bool redo();
    bool canUndo() const { return !undo_.empty(); }
    bool canRedo() const { return !redo_.empty(); }
    bool requestHint();
    void giveUp();                               // fills in the solution (recursive solver)
    void setPaused(bool p);
    void setNotesMode(bool on) { notesMode_ = on; }
    bool notesMode() const { return notesMode_; }

    // ---- statistics ----
    int  mistakes() const { return mistakes_; }
    int  hintsUsed() const { return hintsUsed_; }
    int  hintsLeft() const { return relaxed_ ? 99 : std::max(0, hintLimit_ - hintsUsed_); }
    int  lastHintCell() const { return lastHintCell_; }
    const std::string& guidance() const { return guidance_; }
    double elapsedSeconds() const;
    int  score() const;
    GameRecord makeRecord(const std::string& player) const;

private:
    void begin(const Puzzle& p, Difficulty d, bool relaxed, bool daily);
    void finish(GameState s);
    void removeNoteFromPeers(int r, int c, int digit, std::vector<NoteChange>& changes);

    bool active_ = false;
    Difficulty difficulty_ = Difficulty::Beginner;
    bool relaxed_ = false, daily_ = false, notesMode_ = false;
    GameState state_ = GameState::Playing;

    SudokuBoard board_;
    int  puzzle_[9][9];
    int  solution_[9][9];
    bool hinted_[9][9];

    Stack<Move> undo_;
    Stack<Move> redo_;
    Queue<Hint> hintQueue_;

    int mistakes_ = 0, hintsUsed_ = 0, hintLimit_ = 2, lastHintCell_ = -1;
    std::string guidance_;

    bool   paused_ = false, running_ = false;
    double accumulated_ = 0.0;
    std::chrono::steady_clock::time_point started_;
    int    finalSeconds_ = 0, finalScore_ = 0;
};
