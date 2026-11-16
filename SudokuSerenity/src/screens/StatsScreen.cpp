#include "screens/StatsScreen.h"
#include <cstdio>
#include "Garden.h"
#include "Util.h"

using namespace ui;

void StatsScreen::onMouseDown(sf::Vector2f p) {
    int tab = navHit(p);
    if (tab >= 0 && tab != 1) navigateTab(ctx_, tab);
}
void StatsScreen::onKey(sf::Keyboard::Key key, bool) { if (key == sf::Keyboard::Key::Escape) ctx_.goTo(ScreenId::Home); }

void StatsScreen::draw(sf::RenderTarget& t) {
    const Fonts& F = ctx_.fonts;
    backdrop(t, ctx_.time);
    const PlayerProfile* p = ctx_.players.current();
    text(t, F.title, "My Progress", 38, {40.f, 24.f}, cfg::INK);
    text(t, F.ui, p ? "Statistics for " + p->name : "No player selected", 16, {42.f, 76.f}, cfg::INK_SOFT);
    if (!p) { navBar(t, F, 1, mouse_); return; }

    // overall tiles
    char avg[16];
    std::snprintf(avg, sizeof(avg), "%.1f", p->averageMistakes());
    struct Tile { const char* label; std::string value; };
    Tile tiles[5] = { { "GAMES PLAYED", std::to_string(p->playedTotal()) }, { "GAMES COMPLETED", std::to_string(p->completedTotal()) },
                      { "HIGHEST SCORE", std::to_string(p->highestScore()) }, { "AVG. MISTAKES", avg },
                      { "TOTAL SCORE", std::to_string(p->totalScore) } };
    for (int i = 0; i < 5; ++i) {
        float x = 40.f + i * 220.f;
        card(t, R(x, 108.f, 204.f, 84.f), 18.f);
        text(t, F.uiBold, tiles[i].label, 12, {x + 102.f, 122.f}, cfg::INK_SOFT, Align::Center);
        text(t, F.numberBold, tiles[i].value, 30, {x + 102.f, 144.f}, cfg::INK, Align::Center);
    }

    // per grid size
    card(t, R(40.f, 208.f, 420.f, 214.f), 20.f);
    text(t, F.title, "By grid size", 24, {62.f, 220.f}, cfg::INK);
    text(t, F.uiBold, "SIZE", 12, {62.f, 262.f}, cfg::INK_SOFT);
    text(t, F.uiBold, "COMPLETED", 12, {200.f, 262.f}, cfg::INK_SOFT);
    text(t, F.uiBold, "BEST TIME", 12, {340.f, 262.f}, cfg::INK_SOFT);
    for (int i = 0; i < SIZE_COUNT; ++i) {
        float y = 288.f + i * 40.f;
        rect(t, R(62.f, y - 6.f, 376.f, 1.f), cfg::SAND);
        text(t, F.numberBold, sizeLabel(i), 18, {62.f, y + 2.f}, cfg::INK);
        text(t, F.number, std::to_string(p->completedForSize(i)), 18, {200.f, y + 2.f}, cfg::INK);
        int bt = p->bestTimeForSize(i);
        text(t, F.number, bt ? util::formatTime(bt) : "--:--", 18, {340.f, y + 2.f}, cfg::INK);
    }

    // per difficulty
    card(t, R(480.f, 208.f, 660.f, 244.f), 20.f);
    text(t, F.title, "By difficulty", 24, {502.f, 220.f}, cfg::INK);
    const char* heads[5] = { "LEVEL", "PLAYED", "COMPLETED", "BEST TIME", "BEST SCORE" };
    float hx[5] = { 502.f, 700.f, 800.f, 930.f, 1040.f };
    for (int i = 0; i < 5; ++i) text(t, F.uiBold, heads[i], 12, {hx[i], 262.f}, cfg::INK_SOFT);
    for (int d = 0; d < DIFFICULTY_COUNT; ++d) {
        float y = 288.f + d * 40.f;
        Difficulty df = difficultyFromInt(d);
        rect(t, R(502.f, y - 6.f, 616.f, 1.f), cfg::SAND);
        text(t, F.uiBold, std::string(difficultyName(df)) + "  " + sizeLabel(sizeIndexOf(df)), 16, {hx[0], y + 2.f}, cfg::INK);
        text(t, F.number, std::to_string(p->played[d]), 17, {hx[1], y + 2.f}, cfg::INK);
        text(t, F.number, std::to_string(p->completed[d]), 17, {hx[2], y + 2.f}, cfg::INK);
        text(t, F.number, p->bestTime[d] ? util::formatTime(p->bestTime[d]) : "--:--", 17, {hx[3], y + 2.f}, cfg::INK);
        text(t, F.number, p->bestScore[d] ? std::to_string(p->bestScore[d]) : "-", 17, {hx[4], y + 2.f}, cfg::INK);
    }

    // garden progress
    int done = p->completedTotal(), stage = gardenStageFor(done);
    card(t, R(40.f, 438.f, 420.f, 190.f), 20.f);
    text(t, F.title, "Your garden", 24, {62.f, 450.f}, cfg::INK);
    text(t, F.ui, std::string("Stage ") + std::to_string(stage) + " of " + std::to_string(GARDEN_MAX_STAGE) + ": " + gardenStageName(stage), 15, {62.f, 486.f}, cfg::INK_SOFT);
    for (int s = 0; s <= GARDEN_MAX_STAGE; ++s) {
        float cx = 72.f + s * 41.f;
        if (s > 0) line(t, {cx - 41.f + 11.f, 552.f}, {cx - 11.f, 552.f}, 2.f, s <= stage ? cfg::SAGE : cfg::SAND);
        if (s <= stage) blossom(t, {cx, 552.f}, 13.f, s * 15.f, cfg::SAKURA);
        else circle(t, {cx, 552.f}, 9.f, cfg::SAND, cfg::GOLD, 1.5f);
    }
    if (stage < GARDEN_MAX_STAGE) {
        int more = gardenThreshold(stage + 1) - done;
        text(t, F.ui, std::to_string(more) + " more puzzle" + (more == 1 ? "" : "s") + " until: " + gardenStageName(stage + 1), 14, {62.f, 586.f}, cfg::INK);
    } else text(t, F.uiBold, "Your garden is in full bloom.", 14, {62.f, 586.f}, cfg::SAGE_DARK);

    // recent games (history linked list)
    card(t, R(480.f, 468.f, 660.f, 160.f), 20.f);
    text(t, F.title, "Recent games", 22, {502.f, 478.f}, cfg::INK);
    std::vector<GameRecord> mine = ctx_.history.forPlayer(p->name);
    if (mine.empty()) text(t, F.ui, "No games yet. Your journey begins with one number.", 15, {502.f, 530.f}, cfg::INK_SOFT);
    for (std::size_t i = 0; i < mine.size() && i < 4; ++i) {
        const GameRecord& r = mine[i];
        float y = 514.f + static_cast<float>(i) * 28.f;
        text(t, F.uiBold, std::string(difficultyName(r.difficulty)) + " " + sizeLabel(sizeIndexOf(r.difficulty)), 15, {502.f, y}, cfg::INK);
        text(t, F.ui, r.completed ? (r.relaxed ? "relaxed" : util::formatTime(r.seconds)) : "not finished", 15, {780.f, y}, cfg::INK_SOFT);
        text(t, F.numberBold, r.completed ? std::to_string(r.score) : "-", 15, {900.f, y}, r.completed ? cfg::SAGE_DARK : cfg::INK_SOFT);
        text(t, F.ui, r.date, 13, {1118.f, y + 2.f}, cfg::INK_SOFT, Align::Right);
    }
    navBar(t, F, 1, mouse_);
}
