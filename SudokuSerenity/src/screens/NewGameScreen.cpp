#include "screens/NewGameScreen.h"
#include "Garden.h"
#include "ScoreManager.h"
#include "Util.h"

using namespace ui;

namespace {
const char* BLURB[4] = {
    "A gentle start. Four numbers, four small boxes, perfect for a warm-up.",
    "A pleasant stretch for the mind with rectangular sub-grids.",
    "The classic nine by nine puzzle. Take your time.",
    "Fewer clues and deeper logic. For a still, focused mind."
};

void miniGrid(sf::RenderTarget& t, sf::Vector2f c, float size, const GridSpec& s) {
    float x0 = c.x - size / 2.f, y0 = c.y - size / 2.f, cell = size / s.n;
    roundRect(t, R(x0 - 6.f, y0 - 6.f, size + 12.f, size + 12.f), 10.f, cfg::WOOD);
    rect(t, R(x0, y0, size, size), cfg::PAPER);
    for (int i = 0; i <= s.n; ++i) {
        bool boxV = (i % s.boxC == 0), boxH = (i % s.boxR == 0);
        rect(t, R(x0 + i * cell - (boxV ? 1.f : 0.5f), y0, boxV ? 2.f : 1.f, size), boxV ? cfg::WOOD_DARK : alpha(cfg::SAGE_DARK, 130));
        rect(t, R(x0, y0 + i * cell - (boxH ? 1.f : 0.5f), size, boxH ? 2.f : 1.f), boxH ? cfg::WOOD_DARK : alpha(cfg::SAGE_DARK, 130));
    }
    rect(t, R(x0 + cell, y0 + cell, cell, cell), alpha(cfg::SAGE, 190));          // a calm "selected" cell
}
}

NewGameScreen::NewGameScreen(AppContext& ctx) : Screen(ctx) {
    backBtn_ = { R(40.f, 654.f, 130.f, 44.f), "Back", true };
    relaxRect_ = R(320.f, 540.f, 540.f, 84.f);
}

void NewGameScreen::start(int level) {
    ctx_.startNew(difficultyFromInt(level));
    ctx_.goTo(ScreenId::Game);
}

void NewGameScreen::onMouseDown(sf::Vector2f p) {
    if (backBtn_.contains(p)) { ctx_.goTo(ScreenId::Home); return; }
    if (relaxRect_.contains(p)) { ctx_.relaxed = !ctx_.relaxed; return; }
    for (int i = 0; i < DIFFICULTY_COUNT; ++i)
        if (cardRect(i).contains(p)) { start(i); return; }
}

void NewGameScreen::onKey(sf::Keyboard::Key key, bool) {
    if (key == sf::Keyboard::Key::Escape) ctx_.goTo(ScreenId::Home);
    if (key == sf::Keyboard::Key::R) ctx_.relaxed = !ctx_.relaxed;
    int n = static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::Num1);
    if (n >= 0 && n < DIFFICULTY_COUNT) start(n);
}

void NewGameScreen::draw(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    backdrop(t, ctx_.time);
    text(t, F.title, "Choose your path", 38, {40.f, 28.f}, cfg::INK);
    text(t, F.ui, "Every puzzle grows your garden.  Press 1-4 to start quickly, or click a card.", 16, {42.f, 80.f}, cfg::INK_SOFT);

    const PlayerProfile* me = ctx_.players.current();
    for (int i = 0; i < DIFFICULTY_COUNT; ++i) {
        Difficulty d = difficultyFromInt(i);
        GridSpec spec = specFor(d);
        sf::FloatRect r = cardRect(i);
        bool h = r.contains(mouse_);
        card(t, R(r.position.x, r.position.y - (h ? 4.f : 0.f), r.size.x, r.size.y), 22.f, cfg::PAPER, h ? 64 : 36);
        float top = r.position.y - (h ? 4.f : 0.f);
        circle(t, {r.position.x + 30.f, top + 30.f}, 15.f, i == 3 ? cfg::SAKURA : (i == 2 ? cfg::SAGE : cfg::SAND));
        textIn(t, F.numberBold, std::to_string(i + 1), 14, R(r.position.x + 15.f, top + 15.f, 30.f, 30.f), cfg::INK);
        miniGrid(t, {r.position.x + r.size.x / 2.f, top + 112.f}, spec.n == 9 ? 108.f : (spec.n == 6 ? 102.f : 92.f), spec);
        text(t, F.title, difficultyName(d), 28, {r.position.x + r.size.x / 2.f, top + 190.f}, cfg::INK, Align::Center);
        text(t, F.uiBold, std::string(sizeLabel(sizeIndexOf(d))) + "  -  " + levelName(d), 15, {r.position.x + r.size.x / 2.f, top + 228.f}, cfg::SAGE_DARK, Align::Center);
        float y = top + 256.f;
        for (const std::string& ln : wrap(F.ui, BLURB[i], 14, r.size.x - 40.f)) { text(t, F.ui, ln, 14, {r.position.x + r.size.x / 2.f, y}, cfg::INK_SOFT, Align::Center); y += 19.f; }
        int best = me ? me->bestTime[i] : 0;
        const std::string rows[3] = { "Clues:  " + std::to_string(cluesFor(d)) + " of " + std::to_string(spec.n * spec.n),
                                      "Par time:  " + util::formatTime(Scoring::parSeconds(d)),
                                      "Your best:  " + (best ? util::formatTime(best) : std::string("--:--")) };
        float ry = top + 316.f;
        for (const std::string& rr : rows) { text(t, F.uiBold, rr, 14, {r.position.x + 28.f, ry}, cfg::INK); ry += 20.f; }
    }

    // relaxation mode switch
    card(t, relaxRect_, 20.f, cfg::PAPER, 36);
    drawIcon(t, Icon::Leaf, {relaxRect_.position.x + 40.f, relaxRect_.position.y + 42.f}, 26.f, cfg::SAGE_DARK);
    text(t, F.title, "Relaxation mode", 24, {relaxRect_.position.x + 76.f, relaxRect_.position.y + 14.f}, cfg::INK);
    text(t, F.ui, "No timer and unlimited guidance. Play at your own pace.", 15, {relaxRect_.position.x + 76.f, relaxRect_.position.y + 50.f}, cfg::INK_SOFT);
    toggle(t, R(relaxRect_.position.x + relaxRect_.size.x - 86.f, relaxRect_.position.y + 26.f, 60.f, 32.f), ctx_.relaxed);

    button(t, F, backBtn_, hover(backBtn_), BtnStyle::Secondary, 17);
    drawIcon(t, Icon::Back, {backBtn_.rect.position.x + 24.f, backBtn_.rect.position.y + 22.f}, 14.f, cfg::INK);
}
