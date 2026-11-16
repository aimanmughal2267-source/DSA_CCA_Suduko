#include "screens/GameScreen.h"
#include <algorithm>
#include <cmath>
#include "Util.h"
#include "ds/Graph.h"

using namespace ui;

GameScreen::GameScreen(AppContext& ctx) : Screen(ctx) {
    const char* labels[6] = { "Undo", "Redo", "Erase", "Notes", "Hint", "Pause" };
    ui::Button* b[6] = { &undoBtn_, &redoBtn_, &eraseBtn_, &notesBtn_, &hintBtn_, &pauseBtn_ };
    for (int i = 0; i < 6; ++i) *b[i] = { R(672.f + i * 79.6f, 452.f, 70.f, 62.f), labels[i], true };
    saveBtn_     = { R(672.f, 530.f, 230.f, 46.f), "Save and Menu", true };
    solutionBtn_ = { R(910.f, 530.f, 230.f, 46.f), "Show Solution", true };
    musicBtn_    = { R(1096.f, 20.f, 44.f, 44.f), "", true };
    resumeBtn_   = { R(250.f, 410.f, 180.f, 48.f), "Resume", true };
    againBtn_    = { R(220.f, 440.f, 190.f, 48.f), "New Puzzle", true };
    menuBtn_     = { R(430.f, 440.f, 190.f, 48.f), "Main Menu", true };
}

void GameScreen::say(const std::string& msg) { toast_ = msg; toastTimer_ = 4.f; }

void GameScreen::layout() {
    int n = ctx_.game.n();
    cell_ = std::min(96.f, std::floor(540.f / static_cast<float>(n)));
    ox_ = 340.f - cell_ * n / 2.f;
    oy_ = 366.f - cell_ * n / 2.f;
    resumeBtn_.rect = R(250.f, 366.f + cell_ * n / 2.f - 70.f, 180.f, 48.f);
}

sf::FloatRect GameScreen::cellRect(int r, int c) const { return R(ox_ + c * cell_, oy_ + r * cell_, cell_, cell_); }

sf::FloatRect GameScreen::padRect(int i) const {
    int n = ctx_.game.n();
    float gap = 8.f;
    float w = std::min(58.f, (468.f - gap * (n - 1)) / n);
    float total = w * n + gap * (n - 1);
    return R(672.f + (468.f - total) / 2.f + i * (w + gap), 380.f, w, 58.f);
}

void GameScreen::onEnter() {
    recorded_ = false; completeT_ = 0.f; toast_.clear(); toastTimer_ = 0.f;
    selR_ = selC_ = -1;
    peers_.assign(81, false);
    if (!ctx_.game.active()) { ctx_.goTo(ScreenId::Home); return; }
    layout();
}

void GameScreen::select(int r, int c) {
    selR_ = r; selC_ = c;
    peers_.assign(81, false);
    if (r < 0) return;
    const SudokuGraph& G = SudokuGraph::forSize(ctx_.game.n());
    for (int v : G.bfsWithin(G.index(r, c), 1)) peers_[static_cast<std::size_t>(v)] = true;   // BFS over the constraint graph
}

void GameScreen::leaveToMenu() {
    ctx_.autosave();
    ctx_.goTo(ScreenId::Home);
}

void GameScreen::enterDigit(int digit) {
    GameSession& g = ctx_.game;
    if (selR_ < 0) { say("Select a square first."); return; }
    if (!g.isEditable(selR_, selC_)) { say("That square is already settled."); return; }
    if (g.input(selR_, selC_, digit) == InputResult::Mistake)
        say("A gentle slip - that number does not belong there.");
}

void GameScreen::update(float dt) {
    pulse_ += dt;
    if (toastTimer_ > 0.f) toastTimer_ -= dt;
    GameSession& g = ctx_.game;
    if (g.active() && g.finished() && !recorded_) {
        recorded_ = true;
        ctx_.finishGame();
        if (g.state() == GameState::Completed) completeT_ = 0.001f;
    }
    if (completeT_ > 0.f) {
        completeT_ += dt;
        if (completeT_ > 3.6f) ctx_.goTo(ScreenId::Result);
    }
}

// ------------------------------- input ---------------------------------------
void GameScreen::onMouseDown(sf::Vector2f p) {
    GameSession& g = ctx_.game;
    if (completeT_ > 0.f) { ctx_.goTo(ScreenId::Result); return; }
    if (musicBtn_.contains(p)) { ctx_.music.toggle(); return; }

    if (g.state() == GameState::GaveUp) {
        if (againBtn_.contains(p)) { ctx_.startNew(g.difficulty()); ctx_.goTo(ScreenId::Game); }
        if (menuBtn_.contains(p)) ctx_.goTo(ScreenId::Home);
        return;
    }
    if (g.paused()) {
        if (resumeBtn_.contains(p) || pauseBtn_.contains(p)) g.setPaused(false);
        if (saveBtn_.contains(p)) leaveToMenu();
        return;
    }
    if (saveBtn_.contains(p)) { leaveToMenu(); return; }
    if (solutionBtn_.contains(p)) { g.giveUp(); return; }
    if (pauseBtn_.contains(p)) { g.setPaused(true); return; }
    if (undoBtn_.contains(p)) { if (!g.undo()) say("Nothing to undo."); return; }
    if (redoBtn_.contains(p)) { if (!g.redo()) say("Nothing to redo."); return; }
    if (eraseBtn_.contains(p)) { if (selR_ >= 0) g.erase(selR_, selC_); return; }
    if (notesBtn_.contains(p)) { g.setNotesMode(!g.notesMode()); return; }
    if (hintBtn_.contains(p)) {
        if (g.requestHint()) { int c = g.lastHintCell(); if (c >= 0) select(c / g.n(), c % g.n()); }
        else say(g.guidance());
        return;
    }
    for (int i = 0; i < g.n(); ++i)
        if (padRect(i).contains(p)) { enterDigit(i + 1); return; }
    for (int r = 0; r < g.n(); ++r)
        for (int c = 0; c < g.n(); ++c)
            if (cellRect(r, c).contains(p)) { select(r, c); return; }
}

void GameScreen::onKey(sf::Keyboard::Key key, bool ctrl) {
    using K = sf::Keyboard::Key;
    GameSession& g = ctx_.game;
    if (completeT_ > 0.f) { if (key == K::Enter || key == K::Space) ctx_.goTo(ScreenId::Result); return; }
    if (key == K::Escape) { leaveToMenu(); return; }
    if (g.finished()) return;
    if (key == K::P) { g.setPaused(!g.paused()); return; }
    if (g.paused()) return;

    int n = g.n();
    int cr = selR_ < 0 ? 0 : selR_, cc = selC_ < 0 ? 0 : selC_;
    if (key == K::Left)  { select(cr, std::max(0, cc - 1)); return; }
    if (key == K::Right) { select(cr, std::min(n - 1, cc + 1)); return; }
    if (key == K::Up)    { select(std::max(0, cr - 1), cc); return; }
    if (key == K::Down)  { select(std::min(n - 1, cr + 1), cc); return; }

    int d  = static_cast<int>(key) - static_cast<int>(K::Num0);
    int pd = static_cast<int>(key) - static_cast<int>(K::Numpad0);
    if (d >= 1 && d <= n)   { enterDigit(d); return; }
    if (pd >= 1 && pd <= n) { enterDigit(pd); return; }
    if (d == 0 || pd == 0 || key == K::Delete || key == K::Backspace) { if (selR_ >= 0) g.erase(selR_, selC_); return; }
    if (key == K::N) { g.setNotesMode(!g.notesMode()); return; }
    if (key == K::H) { if (g.requestHint()) { int c = g.lastHintCell(); if (c >= 0) select(c / n, c % n); } else say(g.guidance()); return; }
    if ((key == K::Z && ctrl) || key == K::U) { if (!g.undo()) say("Nothing to undo."); return; }
    if (key == K::Y) { if (!g.redo()) say("Nothing to redo."); return; }
}

// ------------------------------- drawing -------------------------------------
void GameScreen::drawBoard(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    const GameSession& g = ctx_.game;
    const int n = g.n();
    const GridSpec& sp = g.spec();
    const float size = cell_ * n;

    // minimalist wooden frame
    sf::FloatRect frame = R(ox_ - 18.f, oy_ - 18.f, size + 36.f, size + 36.f);
    for (int i = 3; i >= 1; --i)
        roundRect(t, R(frame.position.x - i, frame.position.y - i + 6, frame.size.x + 2.f * i, frame.size.y + 2.f * i), 22.f + i, alpha(cfg::INK, 16));
    roundRect(t, frame, 22.f, cfg::WOOD, cfg::WOOD_DARK, 1.5f);
    for (int i = 0; i < 14; ++i) {                                   // subtle grain
        float y = frame.position.y + 4.f + i * (frame.size.y - 8.f) / 13.f;
        line(t, {frame.position.x + 8.f, y + std::sin(i * 1.7f) * 2.f}, {frame.position.x + frame.size.x - 8.f, y + std::cos(i * 1.3f) * 2.f}, 1.f, alpha(cfg::WOOD_DARK, 48));
    }
    rect(t, R(ox_ - 3.f, oy_ - 3.f, size + 6.f, size + 6.f), cfg::PAPER, cfg::WOOD_DARK, 1.f);
    grain(t, R(ox_, oy_, size, size), 40, 12, 3);

    int selVal = (selR_ >= 0) ? g.value(selR_, selC_) : 0;
    bool showHighlights = !g.paused();

    for (int r = 0; r < n; ++r)
        for (int c = 0; c < n; ++c) {
            sf::FloatRect cr = cellRect(r, c);
            int v = g.value(r, c);
            if (showHighlights) {
                if (g.rowComplete(r) || g.colComplete(c) || g.boxComplete(r, c)) rect(t, cr, alpha(cfg::GOLD, 40));   // finished lines glow gold
                if (selR_ >= 0 && peers_[static_cast<std::size_t>(r * n + c)]) rect(t, cr, alpha(cfg::SAGE, 46));
                if (v != 0 && v == selVal) rect(t, cr, alpha(cfg::GOLD, 120));                                          // same number
                if (g.isWrong(r, c)) rect(t, cr, alpha(cfg::SAKURA, 190));                                              // incorrect: pale pink
                if (r == selR_ && c == selC_) rect(t, cr, alpha(cfg::SAGE, 200));                                       // selected: sage
            }
        }
    // lines
    for (int i = 0; i <= n; ++i) {
        bool vBox = (i % sp.boxC == 0), hBox = (i % sp.boxR == 0);
        rect(t, R(ox_ + i * cell_ - (vBox ? 1.2f : 0.5f), oy_, vBox ? 2.4f : 1.f, size), vBox ? cfg::WOOD_DARK : alpha(cfg::SAGE_DARK, 120));
        rect(t, R(ox_, oy_ + i * cell_ - (hBox ? 1.2f : 0.5f), size, hBox ? 2.4f : 1.f), hBox ? cfg::WOOD_DARK : alpha(cfg::SAGE_DARK, 120));
    }
    // numbers and notes
    unsigned numSize = static_cast<unsigned>(cell_ * 0.52f);
    unsigned noteSize = static_cast<unsigned>(std::max(11.f, cell_ * 0.2f));
    for (int r = 0; r < n; ++r)
        for (int c = 0; c < n; ++c) {
            sf::FloatRect cr = cellRect(r, c);
            int v = g.value(r, c);
            if (v != 0) {
                sf::Color col; const sf::Font* f = &F.number;
                if (g.isGiven(r, c))       { col = cfg::INK; f = &F.numberBold; }
                else if (g.isHinted(r, c)) { col = cfg::WOOD_DARK; f = &F.numberBold; }
                else if (g.isWrong(r, c))  { col = cfg::SAKURA_DARK; f = &F.numberBold; }
                else                       { col = mix(cfg::SAGE_DARK, cfg::INK, 0.35f); }
                if (!g.paused()) textIn(t, *f, std::to_string(v), numSize, cr, col);
            } else if (g.notes(r, c) != 0 && !g.paused()) {
                for (int d = 1; d <= n; ++d) {
                    if (!(g.notes(r, c) & (1u << d))) continue;
                    int rr = (d - 1) / sp.boxC, cc2 = (d - 1) % sp.boxC;
                    sf::FloatRect sub = R(cr.position.x + cc2 * cell_ / sp.boxC, cr.position.y + rr * cell_ / sp.boxR, cell_ / sp.boxC, cell_ / sp.boxR);
                    textIn(t, F.number, std::to_string(d), noteSize, sub, cfg::INK_SOFT);
                }
            }
        }
    if (selR_ >= 0 && !g.paused())
        roundRect(t, R(ox_ + selC_ * cell_ + 2.f, oy_ + selR_ * cell_ + 2.f, cell_ - 4.f, cell_ - 4.f), 4.f, sf::Color::Transparent, cfg::SAGE_DARK, 2.5f);
    int hc = g.lastHintCell();
    if (hc >= 0 && !g.paused()) {
        std::uint8_t a = static_cast<std::uint8_t>(140 + 100 * std::sin(pulse_ * 3.f));
        roundRect(t, R(ox_ + (hc % n) * cell_ + 5.f, oy_ + (hc / n) * cell_ + 5.f, cell_ - 10.f, cell_ - 10.f), 6.f, sf::Color::Transparent, alpha(cfg::GOLD, a), 2.f);
    }
    if (g.paused()) {
        rect(t, R(ox_, oy_, size, size), cfg::PAPER);
        grain(t, R(ox_, oy_, size, size), 60, 14, 5);
        float cx = ox_ + size / 2.f, cy = oy_ + size / 2.f;
        ensoRing(t, {cx, cy - 50.f}, 96.f, 5.f, alpha(cfg::SAGE, 180));
        text(t, F.title, "Take a", 30, {cx, cy - 86.f}, cfg::INK, Align::Center);
        text(t, F.title, "breath", 30, {cx, cy - 50.f}, cfg::INK, Align::Center);
        text(t, F.ui, "The puzzle is resting.", 16, {cx, cy + 66.f}, cfg::INK_SOFT, Align::Center);
    }
}

void GameScreen::drawPanel(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    GameSession& g = ctx_.game;
    const float X = 672.f, W = 468.f;

    text(t, F.title, difficultyName(g.difficulty()), 36, {X, 18.f}, cfg::INK);
    std::string sub = std::string(sizeLabel(sizeIndexOf(g.difficulty()))) + "  -  " + levelName(g.difficulty());
    if (g.daily()) sub += "  -  Daily Challenge";
    text(t, F.ui, sub, 16, {X + 2.f, 66.f}, cfg::INK_SOFT);
    iconButton(t, F, musicBtn_, hover(musicBtn_), Icon::Music, ctx_.music.isOn());

    // time / relaxation + score
    card(t, R(X, 100.f, 226.f, 80.f), 16.f);
    text(t, F.uiBold, g.relaxed() ? "RELAXATION" : "TIME", 12, {X + 16.f, 112.f}, cfg::INK_SOFT);
    if (g.relaxed()) { drawIcon(t, Icon::Leaf, {X + 40.f, 150.f}, 24.f, cfg::SAGE_DARK); text(t, F.ui, "No timer. Breathe.", 15, {X + 62.f, 140.f}, cfg::INK); }
    else text(t, F.numberBold, util::formatTime(static_cast<int>(g.elapsedSeconds())), 32, {X + 16.f, 128.f}, cfg::INK);
    card(t, R(X + 242.f, 100.f, 226.f, 80.f), 16.f);
    text(t, F.uiBold, "SCORE", 12, {X + 258.f, 112.f}, cfg::INK_SOFT);
    text(t, F.numberBold, std::to_string(g.score()), 32, {X + 258.f, 128.f}, cfg::INK);

    // mistakes
    card(t, R(X, 192.f, W, 64.f), 16.f);
    text(t, F.uiBold, "MISTAKES", 12, {X + 16.f, 204.f}, cfg::INK_SOFT);
    text(t, F.numberBold, std::to_string(g.mistakes()), 28, {X + 16.f, 218.f}, cfg::INK);
    for (int i = 0; i < std::min(g.mistakes(), 12); ++i) petal(t, {X + 80.f + i * 26.f, 232.f}, 20.f, 25.f * i, cfg::SAKURA);

    // guidance
    card(t, R(X, 268.f, W, 98.f), 16.f);
    text(t, F.uiBold, "GUIDANCE", 12, {X + 16.f, 280.f}, cfg::INK_SOFT);
    std::string msg = g.guidance().empty() ? "Need a gentle nudge? Press Hint and a calm suggestion will appear." : g.guidance();
    float y = 302.f;
    for (const std::string& ln : wrap(F.ui, msg, 15, W - 36.f)) { if (y > 346.f) break; text(t, F.ui, ln, 15, {X + 16.f, y}, cfg::INK); y += 20.f; }

    // number pad (adapts to the grid size)
    for (int i = 0; i < g.n(); ++i) {
        sf::FloatRect r = padRect(i);
        int remaining = g.n() - g.digitsPlaced(i + 1);
        bool enabled = !g.finished() && !g.paused() && remaining > 0;
        bool h = enabled && r.contains(mouse_);
        sf::Color fill = !enabled ? mix(cfg::SAND, cfg::IVORY, 0.5f) : (g.notesMode() ? cfg::SAKURA_LIGHT : (h ? cfg::SAGE : cfg::SAGE_LIGHT));
        roundRect(t, R(r.position.x, r.position.y + 2, r.size.x, r.size.y), 14.f, alpha(cfg::INK, 22));
        roundRect(t, r, 14.f, fill, enabled ? cfg::SAGE_DARK : cfg::SAND, 1.5f);
        textIn(t, F.numberBold, std::to_string(i + 1), 24, R(r.position.x, r.position.y + 2.f, r.size.x, r.size.y - 16.f), enabled ? cfg::INK : alpha(cfg::INK, 90));
        text(t, F.ui, std::to_string(remaining) + " left", 10, {r.position.x + r.size.x / 2.f, r.position.y + r.size.y - 16.f}, enabled ? cfg::INK_SOFT : alpha(cfg::INK, 90), Align::Center);
    }

    bool live = !g.finished() && !g.paused();
    undoBtn_.enabled = live && g.canUndo();
    redoBtn_.enabled = live && g.canRedo();
    eraseBtn_.enabled = live;
    notesBtn_.enabled = live;
    hintBtn_.enabled = live;
    pauseBtn_.enabled = !g.finished();
    notesBtn_.label = g.notesMode() ? "Notes: On" : "Notes";
    hintBtn_.label = g.relaxed() ? "Hint" : "Hint (" + std::to_string(g.hintsLeft()) + ")";
    pauseBtn_.label = g.paused() ? "Resume" : "Pause";
    iconButton(t, F, undoBtn_, hover(undoBtn_), Icon::Undo);
    iconButton(t, F, redoBtn_, hover(redoBtn_), Icon::Redo);
    iconButton(t, F, eraseBtn_, hover(eraseBtn_), Icon::Eraser);
    iconButton(t, F, notesBtn_, hover(notesBtn_), Icon::Pencil, g.notesMode());
    iconButton(t, F, hintBtn_, hover(hintBtn_), Icon::Bulb);
    iconButton(t, F, pauseBtn_, hover(pauseBtn_), g.paused() ? Icon::Play : Icon::Pause, g.paused());

    saveBtn_.enabled = !g.finished();
    solutionBtn_.enabled = live;
    button(t, F, saveBtn_, hover(saveBtn_), BtnStyle::Secondary, 16);
    button(t, F, solutionBtn_, hover(solutionBtn_), BtnStyle::Ghost, 16);

    if (toastTimer_ > 0.f) text(t, F.uiBold, toast_, 15, {X, 594.f}, cfg::SAKURA_DARK);
    text(t, F.ui, "1-" + std::to_string(g.n()) + " place   N notes   H hint   Ctrl+Z undo   Y redo   P pause   Esc save and menu",
         13, {X, 686.f}, alpha(cfg::INK, 150));
}

void GameScreen::drawCelebration(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    float a = std::min(1.f, completeT_ / 0.8f);
    sf::Vector2f c{340.f, 366.f};
    for (int k = 0; k < 4; ++k) {                                      // expanding circular ripples
        float rt = completeT_ - k * 0.35f;
        if (rt <= 0.f) continue;
        float rad = 40.f + rt * 190.f;
        ensoRing(t, c, rad, 4.f, alpha(k % 2 ? cfg::SAKURA : cfg::SAGE, static_cast<std::uint8_t>(std::max(0.f, 200.f - rt * 70.f))));
    }
    rect(t, R(0, 0, cfg::WIN_W, cfg::WIN_H), alpha(cfg::IVORY, static_cast<std::uint8_t>(120 * a)));
    fallingPetals(t, ctx_.time * 2.f, 46, R(0, -40.f, cfg::WIN_W, cfg::WIN_H), static_cast<std::uint8_t>(210 * a));
    for (int i = 0; i < 6; ++i) {
        float ang = i * 60.f + completeT_ * 25.f;
        float rad = ang * 3.14159f / 180.f, dist = 130.f + 30.f * std::sin(completeT_ * 2.f + i);
        blossom(t, {c.x + std::cos(rad) * dist * 1.9f, c.y + std::sin(rad) * dist * 1.1f}, 16.f * a, ang, cfg::SAKURA, static_cast<std::uint8_t>(230 * a));
    }
    card(t, R(150.f, 300.f, 380.f, 130.f), 26.f, alpha(cfg::PAPER, static_cast<std::uint8_t>(245 * a)), static_cast<std::uint8_t>(50 * a));
    text(t, F.title, "Beautifully Solved!", 36, {340.f, 322.f}, alpha(cfg::INK, static_cast<std::uint8_t>(255 * a)), Align::Center);
    text(t, F.ui, "Every number found its place.", 17, {340.f, 376.f}, alpha(cfg::INK_SOFT, static_cast<std::uint8_t>(255 * a)), Align::Center);
}

void GameScreen::drawGaveUp(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    rect(t, R(0, 0, cfg::WIN_W, cfg::WIN_H), alpha(cfg::INK, 70));
    card(t, R(200.f, 300.f, 440.f, 210.f), 24.f);
    text(t, F.title, "The solution", 32, {420.f, 322.f}, cfg::INK, Align::Center);
    text(t, F.ui, "Rest a moment. A new puzzle is always waiting.", 16, {420.f, 372.f}, cfg::INK_SOFT, Align::Center);
    againBtn_.rect = R(224.f, 430.f, 190.f, 48.f);
    menuBtn_.rect  = R(426.f, 430.f, 190.f, 48.f);
    button(t, F, againBtn_, hover(againBtn_), BtnStyle::Primary, 17);
    button(t, F, menuBtn_, hover(menuBtn_), BtnStyle::Secondary, 17);
}

void GameScreen::draw(sf::RenderTarget& t) {
    layout();
    backdrop(t, ctx_.time);
    drawBoard(t);
    drawPanel(t);
    if (ctx_.game.paused()) {
        resumeBtn_.rect = R(250.f, 366.f + cell_ * ctx_.game.n() / 2.f - 70.f, 180.f, 48.f);
        button(t, ctx_.fonts, resumeBtn_, hover(resumeBtn_), BtnStyle::Primary, 17);
    }
    if (ctx_.game.state() == GameState::GaveUp) drawGaveUp(t);
    if (completeT_ > 0.f) drawCelebration(t);
}
