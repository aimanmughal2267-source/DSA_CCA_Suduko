#include "screens/ResultScreen.h"
#include <algorithm>
#include "Achievements.h"
#include "Garden.h"
#include "Util.h"

using namespace ui;

ResultScreen::ResultScreen(AppContext& ctx) : Screen(ctx) {
    nextBtn_ = { R(380.f, 610.f, 200.f, 50.f), "Next Puzzle", true };
    menuBtn_ = { R(600.f, 610.f, 200.f, 50.f), "Main Menu", true };
}

void ResultScreen::nextPuzzle() {
    if (!ctx_.result.valid) { ctx_.goTo(ScreenId::NewGame); return; }
    ctx_.startNew(ctx_.result.rec.difficulty);
    ctx_.goTo(ScreenId::Game);
}

void ResultScreen::onMouseDown(sf::Vector2f p) {
    if (nextBtn_.contains(p)) nextPuzzle();
    else if (menuBtn_.contains(p)) ctx_.goTo(ScreenId::Home);
}

void ResultScreen::onKey(sf::Keyboard::Key key, bool) {
    if (key == sf::Keyboard::Key::Enter) nextPuzzle();
    if (key == sf::Keyboard::Key::Escape) ctx_.goTo(ScreenId::Home);
}

void ResultScreen::draw(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    const ResultInfo& R_ = ctx_.result;
    backdrop(t, ctx_.time);
    fallingPetals(t, ctx_.time, 26, ui::R(0, 0, cfg::WIN_W, cfg::WIN_H), 130);
    if (!R_.valid) { text(t, F.title, "Nothing to show yet.", 30, {590.f, 300.f}, cfg::INK, Align::Center); button(t, F, menuBtn_, hover(menuBtn_), BtnStyle::Secondary); return; }

    const GameRecord& rec = R_.rec;
    float appear = std::min(1.f, (ctx_.time - enterTime_) / 0.6f);
    card(t, ui::R(210.f, 28.f, 760.f, 664.f), 30.f, cfg::PAPER, 56);
    blossom(t, {590.f, 76.f}, 26.f * appear, ctx_.time * 12.f, cfg::SAKURA);
    text(t, F.title, "Beautifully Solved!", 44, {590.f, 110.f}, cfg::INK, Align::Center);
    text(t, F.ui, "Every number found its place. Well done!", 18, {590.f, 168.f}, cfg::INK_SOFT, Align::Center);

    // statistic tiles
    struct Tile { std::string label, value; };
    Tile tiles[6] = {
        { "GRID SIZE", sizeLabel(sizeIndexOf(rec.difficulty)) },
        { "DIFFICULTY", std::string(difficultyName(rec.difficulty)) },
        { "TIME", rec.relaxed ? std::string("Relaxed") : util::formatTime(rec.seconds) },
        { "FINAL SCORE", std::to_string(rec.score) },
        { "MISTAKES", std::to_string(rec.mistakes) },
        { "HINTS USED", std::to_string(rec.hints) },
    };
    for (int i = 0; i < 6; ++i) {
        float x = 246.f + (i % 3) * 236.f, y = 212.f + (i / 3) * 92.f;
        roundRect(t, ui::R(x, y, 220.f, 80.f), 16.f, i == 3 ? cfg::SAGE_LIGHT : cfg::IVORY, cfg::SAND, 1.5f);
        text(t, F.uiBold, tiles[i].label, 12, {x + 110.f, y + 12.f}, cfg::INK_SOFT, Align::Center);
        text(t, F.numberBold, tiles[i].value, 26, {x + 110.f, y + 34.f}, cfg::INK, Align::Center);
    }

    float y = 404.f;
    if (R_.best.time)  { drawIcon(t, Icon::Check, {250.f, y + 11.f}, 16.f, cfg::SAGE_DARK); text(t, F.uiBold, "New personal best time!", 16, {268.f, y}, cfg::INK); y += 28.f; }
    if (R_.best.score) { drawIcon(t, Icon::Check, {250.f, y + 11.f}, 16.f, cfg::SAGE_DARK); text(t, F.uiBold, "New personal best score!", 16, {268.f, y}, cfg::INK); y += 28.f; }
    if (rec.daily)     { drawIcon(t, Icon::Flag, {250.f, y + 11.f}, 16.f, cfg::SAKURA_DARK); text(t, F.uiBold, "Daily Challenge completed (+15% score)", 16, {268.f, y}, cfg::INK); y += 28.f; }
    if (R_.rank > 0)   { drawIcon(t, Icon::Trophy, {250.f, y + 11.f}, 16.f, cfg::GOLD); text(t, F.uiBold, "Leaderboard position: #" + std::to_string(R_.rank) + " of " + std::to_string(R_.rankTotal), 16, {268.f, y}, cfg::INK); y += 28.f; }
    if (R_.gardenAfter > R_.gardenBefore) {
        blossom(t, {250.f, y + 11.f}, 9.f, 0.f, cfg::SAKURA);
        text(t, F.uiBold, std::string("Your garden grew: ") + gardenStageName(R_.gardenAfter) + " appeared.", 16, {268.f, y}, cfg::INK);
        y += 28.f;
    }

    // achievements earned in this game
    int shown = 0;
    for (int i = 0; i < ACH_COUNT; ++i) if (R_.newAchievements & (1u << i)) ++shown;
    if (shown > 0) {
        text(t, F.uiBold, "ACHIEVEMENTS EARNED", 12, {590.f, 520.f}, cfg::INK_SOFT, Align::Center);
        float x0 = 590.f - (std::min(shown, 5) - 1) * 70.f;
        int k = 0;
        for (int i = 0; i < ACH_COUNT && k < 5; ++i)
            if (R_.newAchievements & (1u << i)) { badge(t, F, {x0 + k * 140.f, 560.f}, 24.f, i, true, false); text(t, F.ui, achievementDef(i).name, 12, {x0 + k * 140.f, 588.f}, cfg::INK, Align::Center); ++k; }
    }
    nextBtn_.rect = ui::R(380.f, 622.f, 200.f, 50.f);
    menuBtn_.rect = ui::R(600.f, 622.f, 200.f, 50.f);
    button(t, F, nextBtn_, hover(nextBtn_), BtnStyle::Primary, 18);
    button(t, F, menuBtn_, hover(menuBtn_), BtnStyle::Secondary, 18);
}
