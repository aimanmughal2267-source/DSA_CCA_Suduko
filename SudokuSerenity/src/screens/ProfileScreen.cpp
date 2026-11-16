#include "screens/ProfileScreen.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include "Achievements.h"
#include "Garden.h"
#include "Util.h"

using namespace ui;

ProfileScreen::ProfileScreen(AppContext& ctx) : Screen(ctx) { addBtn_ = { R(340.f, 462.f, 110.f, 44.f), "Add", true }; }

void ProfileScreen::onEnter() {
    names_ = ctx_.players.names();
    scroll_ = 0; message_.clear(); input_.clear(); typing_ = true;
}

void ProfileScreen::addPlayer() {
    std::string clean = util::cleanName(input_);
    if (clean.empty()) { message_ = "Please type a name first."; return; }
    if (ctx_.players.find(clean)) { message_ = "That name is already taken."; return; }
    ctx_.autosave();                                               // keep the previous player's running game
    ctx_.players.add(clean, avatar_);
    names_ = ctx_.players.names();
    input_.clear(); message_ = "Welcome, " + clean + "!";
    ctx_.goTo(ScreenId::Home);
}

void ProfileScreen::onText(char32_t ch) {
    if (!typing_) return;
    if (ch == 8) { if (!input_.empty()) input_.pop_back(); }
    else if (ch >= 32 && ch < 127 && input_.size() < 14) {
        char c = static_cast<char>(ch);
        if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '_' || c == '-' || c == '.') input_.push_back(c);
    }
    message_.clear();
}

void ProfileScreen::onKey(sf::Keyboard::Key key, bool) {
    if (key == sf::Keyboard::Key::Enter) addPlayer();
    if (key == sf::Keyboard::Key::Escape && ctx_.players.current()) ctx_.goTo(ScreenId::Home);
}

void ProfileScreen::onWheel(float delta) {
    int maxScroll = std::max(0, static_cast<int>(names_.size()) - ROWS);
    scroll_ = std::max(0, std::min(maxScroll, scroll_ + (delta > 0 ? -1 : 1)));
}

void ProfileScreen::onMouseDown(sf::Vector2f p) {
    int nav = navHit(p);
    if (nav >= 0 && ctx_.players.current()) { if (nav != 3) navigateTab(ctx_, nav); return; }
    if (addBtn_.contains(p)) { addPlayer(); return; }
    typing_ = R(60.f, 462.f, 270.f, 44.f).contains(p);
    for (int i = 0; i < 6; ++i) if (avatarRect(i).contains(p)) { avatar_ = i; return; }
    for (int i = 0; i < ROWS; ++i) {
        int idx = scroll_ + i;
        if (idx < static_cast<int>(names_.size()) && rowRect(i).contains(p)) {
            ctx_.autosave();                                       // keep the previous player's running game
            ctx_.players.switchTo(names_[static_cast<std::size_t>(idx)]);
            return;
        }
    }
}

void ProfileScreen::draw(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    backdrop(t, ctx_.time);
    bool first = ctx_.players.count() == 0;
    text(t, F.title, first ? "Welcome to Sudoku Serenity" : "Profile", 38, {40.f, 24.f}, cfg::INK);
    text(t, F.ui, first ? "Create your first player to begin." : "Switch between players or add a new one.", 16, {42.f, 76.f}, cfg::INK_SOFT);

    // ---- players list + add player ----
    card(t, R(40.f, 108.f, 430.f, 510.f), 22.f);
    text(t, F.title, "Players", 24, {62.f, 118.f}, cfg::INK);
    const PlayerProfile* cur = ctx_.players.current();
    if (names_.empty()) text(t, F.ui, "No players yet.", 16, {62.f, 170.f}, cfg::INK_SOFT);
    for (int i = 0; i < ROWS; ++i) {
        int idx = scroll_ + i;
        if (idx >= static_cast<int>(names_.size())) break;
        const std::string& nm = names_[static_cast<std::size_t>(idx)];
        const PlayerProfile* pp = ctx_.players.find(nm);
        sf::FloatRect r = rowRect(i);
        bool isCur = cur && util::lower(cur->name) == util::lower(nm);
        roundRect(t, r, 16.f, isCur ? cfg::SAGE_LIGHT : (r.contains(mouse_) ? alpha(cfg::SAND, 200) : cfg::IVORY), isCur ? cfg::SAGE : cfg::SAND, 1.5f);
        avatar(t, F, {r.position.x + 28.f, r.position.y + 25.f}, 18.f, pp ? pp->avatar : 0, nm);
        text(t, F.uiBold, nm, 17, {r.position.x + 58.f, r.position.y + 8.f}, cfg::INK);
        text(t, F.ui, std::to_string(pp ? pp->completedTotal() : 0) + " wins" + (isCur ? "  -  active" : ""), 13, {r.position.x + 58.f, r.position.y + 29.f}, cfg::INK_SOFT);
    }
    if (static_cast<int>(names_.size()) > ROWS) text(t, F.ui, "scroll for more", 12, {450.f, 122.f}, alpha(cfg::INK, 120), Align::Right);

    rect(t, R(62.f, 424.f, 386.f, 1.f), cfg::SAND);
    text(t, F.title, "Add player", 22, {62.f, 432.f}, cfg::INK);
    sf::FloatRect box = R(60.f, 462.f, 270.f, 44.f);
    roundRect(t, box, 22.f, cfg::PAPER, typing_ ? cfg::SAGE_DARK : cfg::SAND, 2.f);
    bool caret = typing_ && std::fmod(ctx_.time, 1.f) < 0.55f;
    text(t, F.number, input_ + (caret ? "|" : ""), 18, {box.position.x + 18.f, box.position.y + 9.f}, cfg::INK);
    if (input_.empty() && !typing_) text(t, F.ui, "player name", 15, {box.position.x + 18.f, box.position.y + 13.f}, alpha(cfg::INK, 110));
    button(t, F, addBtn_, hover(addBtn_), BtnStyle::Primary, 17);
    text(t, F.uiBold, "AVATAR", 12, {62.f, 516.f}, cfg::INK_SOFT);
    for (int i = 0; i < 6; ++i) {
        sf::FloatRect r = avatarRect(i);
        if (i == avatar_) circle(t, {r.position.x + 26.f, r.position.y + 26.f}, 29.f, cfg::SAGE_LIGHT, cfg::SAGE_DARK, 2.f);
        avatar(t, F, {r.position.x + 26.f, r.position.y + 26.f}, 22.f, i, input_.empty() ? "A" : input_);
    }
    if (!message_.empty()) text(t, F.uiBold, message_, 14, {62.f, 600.f}, cfg::SAKURA_DARK);

    // ---- selected player's profile ----
    card(t, R(490.f, 108.f, 650.f, 510.f), 22.f);
    if (!cur) {
        text(t, F.title, "No player selected", 28, {815.f, 330.f}, cfg::INK_SOFT, Align::Center);
    } else {
        avatar(t, F, {550.f, 172.f}, 40.f, cur->avatar, cur->name);
        text(t, F.title, cur->name, 34, {612.f, 138.f}, cfg::INK);
        text(t, F.ui, std::string("Garden: ") + gardenStageName(gardenStageFor(cur->completedTotal())), 15, {614.f, 186.f}, cfg::INK_SOFT);

        int best = 0;
        for (int i = 0; i < SIZE_COUNT; ++i) { int b = cur->bestTimeForSize(i); if (b > 0 && (best == 0 || b < best)) best = b; }
        struct Tile { const char* label; std::string value; };
        Tile tiles[4] = { { "GAMES PLAYED", std::to_string(cur->playedTotal()) }, { "WINS", std::to_string(cur->completedTotal()) },
                          { "BEST TIME", best ? util::formatTime(best) : "--:--" }, { "HIGHEST SCORE", std::to_string(cur->highestScore()) } };
        for (int i = 0; i < 4; ++i) {
            float x = 512.f + i * 157.f;
            roundRect(t, R(x, 228.f, 146.f, 70.f), 16.f, cfg::IVORY, cfg::SAND, 1.5f);
            text(t, F.uiBold, tiles[i].label, 11, {x + 73.f, 238.f}, cfg::INK_SOFT, Align::Center);
            text(t, F.numberBold, tiles[i].value, 22, {x + 73.f, 258.f}, cfg::INK, Align::Center);
        }
        text(t, F.title, "Player progress", 20, {512.f, 314.f}, cfg::INK);
        for (int i = 0; i < SIZE_COUNT; ++i) {
            float x = 512.f + i * 207.f;
            roundRect(t, R(x, 346.f, 196.f, 54.f), 14.f, cfg::SAGE_LIGHT, cfg::SAGE, 1.f);
            text(t, F.numberBold, sizeLabel(i), 16, {x + 14.f, 352.f}, cfg::INK);
            int bt = cur->bestTimeForSize(i);
            text(t, F.ui, std::to_string(cur->completedForSize(i)) + " won  -  best " + (bt ? util::formatTime(bt) : "--:--"), 13, {x + 14.f, 376.f}, cfg::INK_SOFT);
        }
        text(t, F.title, "Achievements", 20, {512.f, 414.f}, cfg::INK);
        int hov = -1;
        for (int i = 0; i < ACH_COUNT; ++i) {
            sf::Vector2f c{556.f + (i % 5) * 126.f, 478.f + (i / 5) * 76.f};
            bool on = (cur->achievements & (1u << i)) != 0;
            badge(t, F, c, 22.f, i, on, true);
            float dx = mouse_.x - c.x, dy = mouse_.y - c.y;
            if (dx * dx + dy * dy < 30.f * 30.f) hov = i;
        }
        if (hov >= 0) text(t, F.ui, std::string(achievementDef(hov).name) + ": " + achievementDef(hov).description, 13, {815.f, 599.f}, cfg::INK_SOFT, Align::Center);
    }
    if (cur) navBar(t, F, 3, mouse_);
}
