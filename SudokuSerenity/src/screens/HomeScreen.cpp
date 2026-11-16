#include "screens/HomeScreen.h"
#include <algorithm>
#include <cmath>
#include "Garden.h"
#include "Util.h"

using namespace ui;

HomeScreen::HomeScreen(AppContext& ctx) : Screen(ctx) {
    for (int i = 0; i < 4; ++i) cards_[i].rect = R(40.f, 190.f + i * 112.f, 440.f, 100.f);
    pillAbout_  = { R(1066.f, 18.f, 74.f, 40.f), "About", true };
    pillRelax_  = { R(886.f, 18.f, 170.f, 40.f), "", true };
    pillMusic_  = { R(756.f, 18.f, 120.f, 40.f), "", true };
    pillPlayer_ = { R(556.f, 18.f, 190.f, 40.f), "", true };
}

void HomeScreen::onEnter() {
    showAbout_ = false;
    saveInfo_ = ctx_.describeSave();
    if (!ctx_.players.current()) ctx_.goTo(ScreenId::Profile);
}

void HomeScreen::activate(int card) {
    switch (card) {
        case 0: if (ctx_.hasSave() && ctx_.resume()) ctx_.goTo(ScreenId::Game); break;
        case 1: ctx_.goTo(ScreenId::NewGame); break;
        case 2: ctx_.startDaily(); ctx_.goTo(ScreenId::Game); break;
        default: ctx_.goTo(ScreenId::Stats); break;
    }
}

void HomeScreen::onMouseDown(sf::Vector2f p) {
    if (showAbout_) { showAbout_ = false; return; }
    if (pillAbout_.contains(p))  { showAbout_ = true; return; }
    if (pillMusic_.contains(p))  { ctx_.music.toggle(); return; }
    if (pillRelax_.contains(p))  { ctx_.relaxed = !ctx_.relaxed; return; }
    if (pillPlayer_.contains(p)) { ctx_.goTo(ScreenId::Profile); return; }
    int tab = navHit(p);
    if (tab > 0) { navigateTab(ctx_, tab); return; }
    for (int i = 0; i < 4; ++i) {
        if (i == 0 && !ctx_.hasSave()) continue;
        if (cards_[i].rect.contains(p)) { activate(i); return; }
    }
}

void HomeScreen::pill(sf::RenderTarget& t, const Button& b, Icon icon, bool on) {
    bool h = hover(b);
    roundRect(t, R(b.rect.position.x, b.rect.position.y + 2, b.rect.size.x, b.rect.size.y), 20.f, alpha(cfg::INK, 22));
    roundRect(t, b.rect, 20.f, on ? cfg::SAGE_LIGHT : alpha(cfg::PAPER, h ? 255 : 230), on ? cfg::SAGE : cfg::SAND, 1.5f);
    drawIcon(t, icon, {b.rect.position.x + 22.f, b.rect.position.y + 20.f}, 16.f, cfg::INK);
}

void HomeScreen::draw(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    PlayerProfile* me = ctx_.players.current();
    int completed = me ? me->completedTotal() : 0;
    int stage = gardenStageFor(completed);

    drawGarden(t, stage, ctx_.time);
    gradient(t, R(0, 0, cfg::WIN_W, 96), alpha(cfg::IVORY, 235), alpha(cfg::IVORY, 0));

    // header: logo, player, settings
    blossom(t, {58.f, 38.f}, 17.f, 8.f, cfg::SAKURA);
    text(t, F.title, "Sudoku Serenity", 30, {86.f, 17.f}, cfg::INK);
    pill(t, pillPlayer_, Icon::User, false);
    if (me) avatar(t, F, {pillPlayer_.rect.position.x + 20.f, pillPlayer_.rect.position.y + 20.f}, 14.f, me->avatar, me->name);
    text(t, F.uiBold, me ? me->name : "Add player", 16, {pillPlayer_.rect.position.x + 42.f, pillPlayer_.rect.position.y + 10.f}, cfg::INK);
    bool musicOn = ctx_.music.isOn();
    pill(t, pillMusic_, Icon::Music, musicOn);
    text(t, F.uiBold, ctx_.music.available() ? (musicOn ? "Music: On" : "Music: Off") : "No audio", 15, {pillMusic_.rect.position.x + 42.f, pillMusic_.rect.position.y + 10.f}, cfg::INK);
    pill(t, pillRelax_, Icon::Leaf, ctx_.relaxed);
    text(t, F.uiBold, ctx_.relaxed ? "Relaxation: On" : "Relaxation: Off", 15, {pillRelax_.rect.position.x + 42.f, pillRelax_.rect.position.y + 10.f}, cfg::INK);
    roundRect(t, pillAbout_.rect, 20.f, alpha(cfg::PAPER, hover(pillAbout_) ? 255 : 230), cfg::SAND, 1.5f);
    textIn(t, F.uiBold, "About", 15, pillAbout_.rect, cfg::INK);

    // welcome
    std::string who = me ? me->name : "friend";
    text(t, F.title, util::greeting() + ", " + who, 36, {40.f, 78.f}, cfg::INK);
    text(t, F.ui, "Take a breath. One quiet number at a time.", 17, {42.f, 128.f}, cfg::INK_SOFT);
    text(t, F.ui, ctx_.relaxed ? "Relaxation mode is on: no timer, no pressure." : "Choose where to begin.", 15, {42.f, 154.f}, ctx_.relaxed ? cfg::SAGE_DARK : cfg::INK_SOFT);

    // menu cards
    PlayerProfile* pp = me;
    std::string subs[4];
    subs[0] = saveInfo_;
    subs[1] = "Beginner, Focused, Advanced or Master";
    subs[2] = std::string("Today: ") + difficultyName(ctx_.todaysDifficulty()) + " " + sizeLabel(sizeIndexOf(ctx_.todaysDifficulty()))
              + (ctx_.dailyDoneToday() ? " - completed" : "");
    if (pp) {
        int best = 0;
        for (int i = 0; i < SIZE_COUNT; ++i) { int b = pp->bestTimeForSize(i); if (b > 0 && (best == 0 || b < best)) best = b; }
        subs[3] = std::to_string(pp->playedTotal()) + " games - " + std::to_string(pp->completedTotal()) + " wins - best " + (best ? util::formatTime(best) : "--:--");
    }
    const char* titles[4] = { "Continue", "New Game", "Daily Challenge", "My Progress" };
    Icon icons[4] = { Icon::Play, Icon::Leaf, Icon::Flag, Icon::Chart };
    for (int i = 0; i < 4; ++i) {
        bool enabled = !(i == 0 && !ctx_.hasSave());
        const sf::FloatRect& r = cards_[i].rect;
        bool h = enabled && r.contains(mouse_);
        card(t, R(r.position.x, r.position.y - (h ? 2.f : 0.f), r.size.x, r.size.y), 20.f, alpha(cfg::PAPER, enabled ? 240 : 190), h ? 60 : 36);
        float cy = r.position.y + r.size.y / 2.f - (h ? 2.f : 0.f);
        circle(t, {r.position.x + 50.f, cy}, 28.f, enabled ? (i == 0 ? cfg::SAGE : (i == 2 ? cfg::SAKURA : (i == 3 ? cfg::SAND : cfg::SAGE_LIGHT))) : cfg::SAND);
        drawIcon(t, icons[i], {r.position.x + 50.f, cy}, 22.f, enabled ? cfg::INK : alpha(cfg::INK, 110));
        text(t, F.title, titles[i], 25, {r.position.x + 96.f, cy - 33.f}, enabled ? cfg::INK : alpha(cfg::INK, 120));
        text(t, F.ui, subs[i], 15, {r.position.x + 96.f, cy + 8.f}, enabled ? cfg::INK_SOFT : alpha(cfg::INK_SOFT, 150));
        if (i == 2 && ctx_.dailyDoneToday()) { circle(t, {r.position.x + r.size.x - 34.f, cy}, 14.f, cfg::SAGE); drawIcon(t, Icon::Check, {r.position.x + r.size.x - 34.f, cy}, 16.f, cfg::PAPER); }
    }

    // garden caption
    std::string cap = "Your garden - " + std::string(gardenStageName(stage)) + " (" + std::to_string(stage) + "/" + std::to_string(GARDEN_MAX_STAGE) + ")";
    float cw = textWidth(F.uiBold, cap, 15) + 36.f;
    sf::FloatRect capR = R(1140.f - cw, 76.f, cw, 34.f);
    roundRect(t, capR, 17.f, alpha(cfg::PAPER, 225), cfg::SAND, 1.5f);
    blossom(t, {capR.position.x + 18.f, capR.position.y + 17.f}, 8.f, 0.f, cfg::SAKURA);
    text(t, F.uiBold, cap, 15, {capR.position.x + 32.f, capR.position.y + 8.f}, cfg::INK);
    if (stage < GARDEN_MAX_STAGE) {
        int more = gardenThreshold(stage + 1) - completed;
        text(t, F.ui, "Complete " + std::to_string(more) + " more puzzle" + (more == 1 ? "" : "s") + " to grow it: " + gardenStageName(stage + 1),
             14, {1140.f, 116.f}, alpha(cfg::INK, 190), Align::Right);
    }

    navBar(t, F, 0, mouse_);

    if (showAbout_) {
        rect(t, R(0, 0, cfg::WIN_W, cfg::WIN_H), alpha(cfg::INK, 120));
        sf::FloatRect r = R(330.f, 130.f, 520.f, 440.f);
        card(t, r, 24.f, cfg::PAPER, 60);
        text(t, F.title, "Sudoku Serenity", 34, {590.f, 156.f}, cfg::INK, Align::Center);
        text(t, F.ui, "A calm Sudoku built with Data Structures & Algorithms", 15, {590.f, 204.f}, cfg::INK_SOFT, Align::Center);
        text(t, F.uiBold, std::string(cfg::COURSE_NAME) + "  (" + cfg::COURSE_CODE + ")", 17, {590.f, 240.f}, cfg::INK, Align::Center);
        float y = 288.f;
        for (int i = 0; i < cfg::NUM_MEMBERS; ++i) {
            if (!cfg::MEMBERS[i].name || !cfg::MEMBERS[i].name[0]) continue;
            text(t, F.uiBold, cfg::MEMBERS[i].name, 18, {380.f, y}, cfg::INK);
            text(t, F.ui, cfg::MEMBERS[i].roll ? cfg::MEMBERS[i].roll : "", 16, {820.f, y + 1.f}, cfg::INK_SOFT, Align::Right);
            rect(t, R(380.f, y + 30.f, 440.f, 1.f), alpha(cfg::SAND, 255));
            y += 40.f;
        }
        text(t, F.ui, "Click anywhere to close", 14, {590.f, 534.f}, alpha(cfg::INK, 150), Align::Center);
    }
}
