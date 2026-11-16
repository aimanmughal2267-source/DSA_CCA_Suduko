#include "screens/LeaderboardScreen.h"
#include <cmath>
#include "Util.h"

using namespace ui;

void LeaderboardScreen::onEnter() { tab_ = -1; query_.clear(); found_.clear(); searched_ = false; typing_ = false; refresh(); }

void LeaderboardScreen::refresh() { rows_ = Leaderboard::build(ctx_.scores, ctx_.players, tab_); }

void LeaderboardScreen::search() {
    searched_ = true;
    found_ = ctx_.players.findExact(query_);                // binary search on the sorted name list
}

void LeaderboardScreen::onMouseDown(sf::Vector2f p) {
    int nav = navHit(p);
    if (nav >= 0 && nav != 2) { navigateTab(ctx_, nav); return; }
    for (int i = 0; i < 4; ++i) if (tabRect(i).contains(p)) { tab_ = i - 1; refresh(); return; }
    if (R(900.f, 586.f, 120.f, 40.f).contains(p)) { search(); return; }
    typing_ = R(620.f, 586.f, 270.f, 40.f).contains(p);
}

void LeaderboardScreen::onKey(sf::Keyboard::Key key, bool) {
    if (key == sf::Keyboard::Key::Escape) ctx_.goTo(ScreenId::Home);
    if (key == sf::Keyboard::Key::Enter && !query_.empty()) search();
}

void LeaderboardScreen::onText(char32_t ch) {
    if (!typing_) return;
    if (ch == 8) { if (!query_.empty()) query_.pop_back(); }
    else if (ch >= 32 && ch < 127 && query_.size() < 14) query_.push_back(static_cast<char>(ch));
    searched_ = false;
}

void LeaderboardScreen::draw(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    backdrop(t, ctx_.time);
    text(t, F.title, "Leaderboard", 38, {40.f, 24.f}, cfg::INK);
    text(t, F.ui, "Every registered player, ranked by their best score.", 16, {42.f, 72.f}, cfg::INK_SOFT);

    const char* tabs[4] = { "All sizes", "4 x 4", "6 x 6", "9 x 9" };
    for (int i = 0; i < 4; ++i) {
        Button b{ tabRect(i), tabs[i], true };
        button(t, F, b, hover(b), (tab_ + 1) == i ? BtnStyle::Primary : BtnStyle::Secondary, 16);
    }

    card(t, R(40.f, 148.f, 1100.f, 424.f), 22.f);
    const char* heads[6] = { "#", "PLAYER", "BEST SCORE", "TIME", "LEVEL", "WINS" };
    float hx[6] = { 70.f, 170.f, 470.f, 650.f, 780.f, 1000.f };
    for (int i = 0; i < 6; ++i) text(t, F.uiBold, heads[i], 12, {hx[i], 162.f}, cfg::INK_SOFT);
    rect(t, R(60.f, 184.f, 1060.f, 1.f), cfg::SAND);

    std::string me = ctx_.players.current() ? util::lower(ctx_.players.current()->name) : "";
    int shown = 0;
    for (std::size_t i = 0; i < rows_.size() && shown < 8; ++i, ++shown) {
        const Ranking& r = rows_[i];
        float y = 192.f + shown * 46.f;
        bool mine = util::lower(r.name) == me;
        if (mine) roundRect(t, R(54.f, y - 2.f, 1072.f, 42.f), 14.f, alpha(cfg::SAGE_LIGHT, 230));
        float cy = y + 19.f;
        if (r.ranked && i < 3) {
            sf::Color medal = i == 0 ? cfg::GOLD : (i == 1 ? sf::Color(0xC9, 0xCC, 0xC4) : sf::Color(0xC7, 0x9A, 0x74));
            circle(t, {84.f, cy}, 14.f, medal, cfg::PAPER, 2.f);
            textIn(t, F.numberBold, std::to_string(i + 1), 15, R(70.f, cy - 14.f, 28.f, 28.f), cfg::INK);
        } else text(t, F.number, r.ranked ? std::to_string(i + 1) : "-", 16, {84.f, y + 8.f}, cfg::INK_SOFT, Align::Center);
        avatar(t, F, {136.f, cy}, 15.f, r.avatar, r.name);
        text(t, F.uiBold, r.name + (mine ? "  (you)" : ""), 17, {170.f, y + 8.f}, cfg::INK);
        if (r.ranked) {
            text(t, F.numberBold, std::to_string(r.bestScore), 17, {470.f, y + 8.f}, cfg::SAGE_DARK);
            text(t, F.number, util::formatTime(r.bestSeconds), 16, {650.f, y + 8.f}, cfg::INK);
            text(t, F.ui, difficultyName(r.difficulty), 15, {780.f, y + 9.f}, cfg::INK_SOFT);
            text(t, F.number, std::to_string(r.wins), 16, {1000.f, y + 8.f}, cfg::INK);
        } else text(t, F.ui, "no completed puzzle yet", 14, {470.f, y + 10.f}, alpha(cfg::INK_SOFT, 170));
    }
    if (rows_.empty()) text(t, F.ui, "No players yet.", 18, {590.f, 340.f}, cfg::INK_SOFT, Align::Center);

    // search by name (binary search)
    text(t, F.uiBold, "FIND A PLAYER", 12, {40.f, 596.f}, cfg::INK_SOFT);
    sf::FloatRect box = R(620.f, 586.f, 270.f, 40.f);
    roundRect(t, box, 20.f, cfg::PAPER, typing_ ? cfg::SAGE_DARK : cfg::SAND, 2.f);
    bool caret = typing_ && std::fmod(ctx_.time, 1.f) < 0.55f;
    text(t, F.number, query_ + (caret ? "|" : ""), 17, {box.position.x + 18.f, box.position.y + 7.f}, cfg::INK);
    if (query_.empty() && !typing_) text(t, F.ui, "click and type a name", 15, {box.position.x + 18.f, box.position.y + 10.f}, alpha(cfg::INK, 110));
    Button sb{ R(900.f, 586.f, 120.f, 40.f), "Search", true };
    button(t, F, sb, hover(sb), BtnStyle::Primary, 16);
    if (searched_) {
        if (found_.empty()) text(t, F.uiBold, "No player named \"" + query_ + "\".", 15, {40.f, 618.f}, cfg::SAKURA_DARK);
        else {
            int pos = Leaderboard::positionOf(rows_, found_);
            const PlayerProfile* pp = ctx_.players.find(found_);
            std::string line = found_ + ": rank #" + std::to_string(pos + 1) + ", " + std::to_string(pp ? pp->completedTotal() : 0) + " wins, best score " + std::to_string(pp ? pp->highestScore() : 0);
            text(t, F.uiBold, line, 15, {40.f, 618.f}, cfg::SAGE_DARK);
        }
    } else {
        std::string stats = std::to_string(ctx_.scores.count()) + " scores recorded (BST height " + std::to_string(ctx_.scores.height()) + ")";
        text(t, F.ui, stats, 13, {40.f, 618.f}, alpha(cfg::INK, 130));
    }
    navBar(t, F, 2, mouse_);
}
