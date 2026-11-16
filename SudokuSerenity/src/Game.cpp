#include "Game.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include "ScoreManager.h"
#include "SudokuSolver.h"
#include "Util.h"
#include "ds/Graph.h"

GameSession::GameSession() {
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) { puzzle_[r][c] = solution_[r][c] = 0; hinted_[r][c] = false; }
}

void GameSession::begin(const Puzzle& p, Difficulty d, bool relaxed, bool daily) {
    difficulty_ = d; relaxed_ = relaxed; daily_ = daily;
    board_.reset(p.spec);
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) { puzzle_[r][c] = p.puzzle[r][c]; solution_[r][c] = p.solution[r][c]; hinted_[r][c] = false; }
    board_.loadPuzzle(puzzle_);

    undo_.clear(); redo_.clear(); hintQueue_.clear();
    for (const Solver::HintStep& s : Solver::buildHintOrder(p.spec, puzzle_, solution_))
        hintQueue_.enqueue(Hint{ s.row, s.col, s.value, s.candidates });

    mistakes_ = 0; hintsUsed_ = 0; lastHintCell_ = -1; guidance_.clear();
    hintLimit_ = hintLimitFor(d);
    state_ = GameState::Playing; paused_ = false; notesMode_ = false;
    accumulated_ = 0.0; running_ = true; started_ = std::chrono::steady_clock::now();
    finalSeconds_ = 0; finalScore_ = 0; active_ = true;
}

void GameSession::newGame(Difficulty d, bool relaxed) {
    std::mt19937 rng = Generator::makeRandomEngine();
    begin(Generator::create(d, rng), d, relaxed, false);
}

void GameSession::newDaily(int dateNumber, bool relaxed) {
    Puzzle p = Generator::createDaily(dateNumber);
    begin(p, Generator::dailyDifficulty(dateNumber), relaxed, true);
}

// ------------------------------ information ----------------------------------
int GameSession::digitsPlaced(int digit) const {
    int k = 0;
    for (int r = 0; r < n(); ++r)
        for (int c = 0; c < n(); ++c)
            if (board_.get(r, c) == digit && solution_[r][c] == digit) ++k;
    return k;
}

bool GameSession::rowComplete(int r) const {
    for (int c = 0; c < n(); ++c) if (board_.get(r, c) != solution_[r][c]) return false;
    return true;
}
bool GameSession::colComplete(int c) const {
    for (int r = 0; r < n(); ++r) if (board_.get(r, c) != solution_[r][c]) return false;
    return true;
}
bool GameSession::boxComplete(int r, int c) const {
    const GridSpec& s = spec();
    int r0 = (r / s.boxR) * s.boxR, c0 = (c / s.boxC) * s.boxC;
    for (int i = 0; i < s.boxR; ++i)
        for (int j = 0; j < s.boxC; ++j)
            if (board_.get(r0 + i, c0 + j) != solution_[r0 + i][c0 + j]) return false;
    return true;
}

double GameSession::elapsedSeconds() const {
    if (state_ != GameState::Playing) return finalSeconds_;
    double t = accumulated_;
    if (running_) t += std::chrono::duration<double>(std::chrono::steady_clock::now() - started_).count();
    return t;
}

int GameSession::score() const {
    if (state_ == GameState::Completed) return finalScore_;
    if (state_ == GameState::GaveUp) return 0;
    return Scoring::compute(difficulty_, static_cast<int>(elapsedSeconds()), mistakes_, hintsUsed_, relaxed_, daily_);
}

void GameSession::finish(GameState s) {
    int secs = static_cast<int>(elapsedSeconds());
    state_ = s; running_ = false; paused_ = false; finalSeconds_ = secs;
    finalScore_ = (s == GameState::Completed)
        ? Scoring::compute(difficulty_, secs, mistakes_, hintsUsed_, relaxed_, daily_) : 0;
}

void GameSession::setPaused(bool p) {
    if (state_ != GameState::Playing || p == paused_) return;
    if (p) {
        accumulated_ += std::chrono::duration<double>(std::chrono::steady_clock::now() - started_).count();
        running_ = false;
    } else {
        started_ = std::chrono::steady_clock::now();
        running_ = true;
    }
    paused_ = p;
}

// -------------------------------- actions ------------------------------------
void GameSession::removeNoteFromPeers(int r, int c, int digit, std::vector<NoteChange>& changes) {
    const SudokuGraph& G = SudokuGraph::forSize(n());
    std::uint16_t bit = static_cast<std::uint16_t>(1u << digit);
    for (int w : G.neighbors(G.index(r, c))) {                    // graph neighbours = row, column, box
        int wr = w / n(), wc = w % n();
        std::uint16_t m = board_.notes(wr, wc);
        if (m & bit) {
            changes.push_back(NoteChange{ w, m });
            board_.setNotes(wr, wc, static_cast<std::uint16_t>(m & ~bit));
        }
    }
}

InputResult GameSession::input(int r, int c, int digit) {
    if (state_ != GameState::Playing || paused_) return InputResult::Ignored;
    if (digit < 1 || digit > n() || !isEditable(r, c)) return InputResult::Ignored;

    Move mv{ r, c, board_.get(r, c), board_.get(r, c), board_.notes(r, c), board_.notes(r, c), {} };

    if (notesMode_) {                                              // pencil mark
        if (board_.get(r, c) != 0) return InputResult::Ignored;
        mv.nextNotes = static_cast<std::uint16_t>(board_.notes(r, c) ^ (1u << digit));
        board_.setNotes(r, c, mv.nextNotes);
        undo_.push(mv); redo_.clear();
        return InputResult::NoteToggled;
    }

    if (board_.get(r, c) == digit) return InputResult::Ignored;
    mv.nextValue = digit;
    mv.nextNotes = 0;
    board_.set(r, c, digit);
    board_.setNotes(r, c, 0);
    removeNoteFromPeers(r, c, digit, mv.neighbourNotes);
    undo_.push(mv); redo_.clear();

    if (digit != solution_[r][c]) { ++mistakes_; return InputResult::Mistake; }
    if (board_.matches(solution_)) { finish(GameState::Completed); return InputResult::Completed; }
    return InputResult::Placed;
}

bool GameSession::erase(int r, int c) {
    if (state_ != GameState::Playing || paused_ || !isEditable(r, c)) return false;
    if (board_.get(r, c) == 0 && board_.notes(r, c) == 0) return false;
    Move mv{ r, c, board_.get(r, c), 0, board_.notes(r, c), 0, {} };
    board_.set(r, c, 0);
    board_.setNotes(r, c, 0);
    undo_.push(mv); redo_.clear();
    return true;
}

bool GameSession::undo() {
    if (state_ != GameState::Playing || paused_ || undo_.empty()) return false;
    Move m = undo_.pop();
    board_.set(m.row, m.col, m.prevValue);
    board_.setNotes(m.row, m.col, m.prevNotes);
    for (const NoteChange& ch : m.neighbourNotes) board_.setNotes(ch.cell / n(), ch.cell % n(), ch.previous);
    redo_.push(m);
    return true;
}

bool GameSession::redo() {
    if (state_ != GameState::Playing || paused_ || redo_.empty()) return false;
    Move m = redo_.pop();
    board_.set(m.row, m.col, m.nextValue);
    board_.setNotes(m.row, m.col, m.nextNotes);
    if (m.nextValue != 0) {
        std::uint16_t bit = static_cast<std::uint16_t>(1u << m.nextValue);
        for (const NoteChange& ch : m.neighbourNotes)
            board_.setNotes(ch.cell / n(), ch.cell % n(), static_cast<std::uint16_t>(ch.previous & ~bit));
    }
    undo_.push(m);
    if (board_.matches(solution_)) finish(GameState::Completed);
    return true;
}

bool GameSession::requestHint() {
    if (state_ != GameState::Playing || paused_) return false;
    if (hintsLeft() <= 0) { guidance_ = "No more guidance for this puzzle. Trust yourself - you can do this."; return false; }
    while (!hintQueue_.empty()) {                                  // process the next pending hint
        Hint h = hintQueue_.dequeue();
        if (board_.isGiven(h.row, h.col) || hinted_[h.row][h.col]) continue;
        if (board_.get(h.row, h.col) == solution_[h.row][h.col]) continue;

        board_.set(h.row, h.col, h.value);
        board_.setNotes(h.row, h.col, 0);
        std::vector<NoteChange> ignored;
        removeNoteFromPeers(h.row, h.col, h.value, ignored);
        hinted_[h.row][h.col] = true;
        ++hintsUsed_;
        lastHintCell_ = h.row * n() + h.col;
        std::string where = "row " + std::to_string(h.row + 1) + ", column " + std::to_string(h.col + 1);
        guidance_ = (h.candidates <= 1)
            ? "Look closely at " + where + ": only the number " + std::to_string(h.value) + " fits there."
            : "Take a breath. At " + where + " the number " + std::to_string(h.value) + " belongs.";
        if (board_.matches(solution_)) finish(GameState::Completed);
        return true;
    }
    guidance_ = "There is no further guidance to give.";
    return false;
}

void GameSession::giveUp() {
    if (state_ != GameState::Playing) return;
    int work[9][9];
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) work[r][c] = puzzle_[r][c];
    if (!Solver::solve(spec(), work))                              // recursive backtracking solver
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 9; ++c) work[r][c] = solution_[r][c];
    for (int r = 0; r < n(); ++r)
        for (int c = 0; c < n(); ++c) { board_.set(r, c, work[r][c]); board_.setNotes(r, c, 0); }
    finish(GameState::GaveUp);
}

GameRecord GameSession::makeRecord(const std::string& player) const {
    GameRecord r;
    r.player = player; r.difficulty = difficulty_;
    r.completed = (state_ == GameState::Completed);
    r.relaxed = relaxed_; r.daily = daily_;
    r.seconds = finalSeconds_; r.mistakes = mistakes_; r.hints = hintsUsed_; r.score = finalScore_;
    return r;
}

// ------------------------------ save / resume --------------------------------
bool GameSession::saveTo(const std::string& path) const {
    if (!active_ || state_ != GameState::Playing) return false;
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    out << "SERENITY_SAVE 1\n" << static_cast<int>(difficulty_) << ' ' << (relaxed_ ? 1 : 0) << ' ' << (daily_ ? 1 : 0) << ' '
        << static_cast<int>(elapsedSeconds()) << ' ' << mistakes_ << ' ' << hintsUsed_ << "\n";
    for (int pass = 0; pass < 4; ++pass) {
        for (int r = 0; r < n(); ++r)
            for (int c = 0; c < n(); ++c) {
                if (pass == 0) out << puzzle_[r][c];
                if (pass == 1) out << solution_[r][c];
                if (pass == 2) out << board_.get(r, c);
                if (pass == 3) out << (hinted_[r][c] ? 1 : 0);
            }
        out << "\n";
    }
    for (int r = 0; r < n(); ++r)
        for (int c = 0; c < n(); ++c) out << board_.notes(r, c) << ' ';
    out << "\n";
    return static_cast<bool>(out);
}

bool GameSession::loadFrom(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    std::string header;
    std::getline(in, header);
    if (header.rfind("SERENITY_SAVE", 0) != 0) return false;
    int d = 0, relaxed = 0, daily = 0, elapsed = 0, mistakes = 0, hints = 0;
    in >> d >> relaxed >> daily >> elapsed >> mistakes >> hints;
    if (d < 0 || d >= DIFFICULTY_COUNT) return false;
    GridSpec s = specFor(difficultyFromInt(d));
    std::string rows[4];
    for (int i = 0; i < 4; ++i) in >> rows[i];
    for (int i = 0; i < 4; ++i) if (static_cast<int>(rows[i].size()) != s.n * s.n) return false;

    Puzzle p;
    p.spec = s;
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) p.puzzle[r][c] = p.solution[r][c] = 0;
    for (int r = 0; r < s.n; ++r)
        for (int c = 0; c < s.n; ++c) {
            p.puzzle[r][c]   = rows[0][static_cast<std::size_t>(r * s.n + c)] - '0';
            p.solution[r][c] = rows[1][static_cast<std::size_t>(r * s.n + c)] - '0';
        }
    begin(p, difficultyFromInt(d), relaxed != 0, daily != 0);
    for (int r = 0; r < s.n; ++r)
        for (int c = 0; c < s.n; ++c) {
            board_.set(r, c, rows[2][static_cast<std::size_t>(r * s.n + c)] - '0');
            hinted_[r][c] = rows[3][static_cast<std::size_t>(r * s.n + c)] == '1';
            int m = 0;
            in >> m;
            board_.setNotes(r, c, static_cast<std::uint16_t>(m));
        }
    mistakes_ = mistakes; hintsUsed_ = hints; accumulated_ = elapsed;
    if (board_.matches(solution_)) return false;                    // nothing left to resume
    return true;
}
