#include "Garden.h"
#include <cmath>
#include "Config.h"
#include "Draw.h"

using namespace ui;

namespace {
const int   THRESHOLDS[GARDEN_MAX_STAGE + 1] = { 0, 1, 2, 3, 5, 7, 9, 12, 15 };
const char* NAMES[GARDEN_MAX_STAGE + 1] = {
    "Raked sand", "Stepping stones", "Koi pond", "Bamboo grove", "Stone lantern",
    "Arched bridge and koi", "Pine tree", "Cherry blossom tree", "Torii gate and falling petals" };
constexpr float PI = 3.14159265f;
const float HORIZON = 430.f;

void rakeLines(sf::RenderTarget& t) {
    for (int i = 0; i < 13; ++i) {
        float y = HORIZON + 22.f + i * 22.f;
        sf::VertexArray va(sf::PrimitiveType::LineStrip, 60);
        for (int k = 0; k < 60; ++k) {
            float x = k * (static_cast<float>(cfg::WIN_W) / 59.f);
            va[static_cast<std::size_t>(k)].position = {x, y + std::sin(x * 0.012f + i * 0.9f) * 3.f * (1.f + i * 0.08f)};
            va[static_cast<std::size_t>(k)].color = alpha(cfg::GOLD, 70);
        }
        t.draw(va);
    }
}

void stone(sf::RenderTarget& t, sf::Vector2f c, float rx, float ry, sf::Color col) {
    ellipse(t, c + sf::Vector2f(3.f, 4.f), rx, ry, alpha(cfg::INK, 40));
    ellipse(t, c, rx, ry, col);
    ellipse(t, c + sf::Vector2f(-rx * 0.2f, -ry * 0.3f), rx * 0.55f, ry * 0.4f, mix(col, sf::Color::White, 0.3f));
}

void bamboo(sf::RenderTarget& t, float time) {
    for (int i = 0; i < 6; ++i) {
        float x = 600.f + i * 24.f, top = 230.f + (i % 3) * 28.f;
        float sway = std::sin(time * 0.8f + i) * 5.f;
        for (int seg = 0; seg < 8; ++seg) {
            float y0 = HORIZON + 12.f - seg * (HORIZON + 12.f - top) / 8.f, y1 = y0 - (HORIZON + 12.f - top) / 8.f;
            float s0 = sway * (static_cast<float>(seg) / 8.f), s1 = sway * (static_cast<float>(seg + 1) / 8.f);
            line(t, {x + s0, y0}, {x + s1, y1}, 8.f, mix(cfg::SAGE, cfg::SAGE_DARK, 0.35f + 0.05f * (i % 2)));
            line(t, {x + s1 - 5.f, y1}, {x + s1 + 5.f, y1}, 2.f, cfg::SAGE_DARK);
        }
        float lx = x + sway;
        for (int k = 0; k < 3; ++k)
            ellipse(t, {lx + (k - 1) * 16.f, top + 12.f + k * 8.f}, 22.f, 5.f, alpha(cfg::SAGE_DARK, 210), (k - 1) * 28.f - 12.f);
    }
}

void lantern(sf::RenderTarget& t, sf::Vector2f b, float time) {
    sf::Color stoneCol = mix(cfg::SAND, cfg::INK_SOFT, 0.35f);
    ellipse(t, b + sf::Vector2f(4.f, 5.f), 38.f, 8.f, alpha(cfg::INK, 40));
    rect(t, R(b.x - 28.f, b.y - 12.f, 56.f, 14.f), stoneCol);
    rect(t, R(b.x - 9.f, b.y - 72.f, 18.f, 62.f), mix(stoneCol, sf::Color::White, 0.12f));
    rect(t, R(b.x - 26.f, b.y - 108.f, 52.f, 38.f), stoneCol);
    float glow = 0.75f + 0.25f * std::sin(time * 2.1f);
    rect(t, R(b.x - 15.f, b.y - 100.f, 30.f, 22.f), alpha(cfg::GOLD, static_cast<std::uint8_t>(230 * glow)));
    circle(t, {b.x, b.y - 89.f}, 30.f, alpha(cfg::GOLD, static_cast<std::uint8_t>(40 * glow)));
    polygon(t, { {b.x - 44.f, b.y - 108.f}, {b.x + 44.f, b.y - 108.f}, {b.x + 20.f, b.y - 136.f}, {b.x - 20.f, b.y - 136.f} }, mix(stoneCol, cfg::INK, 0.25f));
    circle(t, {b.x, b.y - 140.f}, 6.f, stoneCol);
}

void pine(sf::RenderTarget& t, sf::Vector2f b, float time) {
    polygon(t, { {b.x - 12.f, b.y}, {b.x + 12.f, b.y}, {b.x + 7.f, b.y - 150.f}, {b.x - 5.f, b.y - 150.f} }, cfg::WOOD_DARK);
    for (int i = 0; i < 4; ++i) {
        float y = b.y - 80.f - i * 42.f, w = 92.f - i * 14.f, sway = std::sin(time * 0.6f + i) * 2.f;
        ellipse(t, {b.x + sway, y + 6.f}, w, 22.f, alpha(cfg::INK, 40));
        ellipse(t, {b.x + sway, y}, w, 22.f, mix(cfg::SAGE_DARK, cfg::INK, 0.25f));
        ellipse(t, {b.x + sway - 8.f, y - 6.f}, w * 0.7f, 14.f, cfg::SAGE_DARK);
    }
}

void cherryTree(sf::RenderTarget& t, sf::Vector2f b, float time) {
    polygon(t, { {b.x - 18.f, b.y}, {b.x + 20.f, b.y}, {b.x + 10.f, b.y - 120.f}, {b.x - 4.f, b.y - 120.f} }, cfg::WOOD_DARK);
    line(t, {b.x + 4.f, b.y - 100.f}, {b.x - 70.f, b.y - 170.f}, 10.f, cfg::WOOD_DARK);
    line(t, {b.x + 6.f, b.y - 110.f}, {b.x + 80.f, b.y - 180.f}, 10.f, cfg::WOOD_DARK);
    line(t, {b.x + 4.f, b.y - 120.f}, {b.x + 6.f, b.y - 200.f}, 9.f, cfg::WOOD_DARK);
    const sf::Vector2f clusters[] = { {-82.f, -186.f}, {-40.f, -218.f}, {8.f, -232.f}, {58.f, -214.f}, {94.f, -192.f}, {-10.f, -180.f}, {40.f, -168.f}, {-60.f, -152.f} };
    int k = 0;
    for (const sf::Vector2f& o : clusters) {
        float pulse = 1.f + 0.03f * std::sin(time * 1.1f + k);
        ellipse(t, {b.x + o.x, b.y + o.y}, 46.f * pulse, 36.f * pulse, alpha(k % 2 ? cfg::SAKURA : cfg::SAKURA_LIGHT, 235));
        ++k;
    }
    for (int i = 0; i < 9; ++i)
        blossom(t, {b.x - 80.f + i * 21.f, b.y - 150.f - std::fmod(i * 37.f, 70.f)}, 7.f, i * 20.f, cfg::PAPER, 200);
}

void torii(sf::RenderTarget& t, sf::Vector2f b) {
    sf::Color red = mix(cfg::SAKURA_DARK, cfg::INK, 0.18f);
    rect(t, R(b.x - 3.f, b.y - 2.f, 90.f, 6.f), alpha(cfg::INK, 40));
    rect(t, R(b.x - 2.f, b.y - 96.f, 11.f, 96.f), red);
    rect(t, R(b.x + 67.f, b.y - 96.f, 11.f, 96.f), red);
    polygon(t, { {b.x - 16.f, b.y - 100.f}, {b.x + 94.f, b.y - 100.f}, {b.x + 84.f, b.y - 88.f}, {b.x - 6.f, b.y - 88.f} }, mix(red, cfg::INK, 0.35f));
    rect(t, R(b.x - 2.f, b.y - 76.f, 80.f, 8.f), red);
}

void pond(sf::RenderTarget& t, sf::Vector2f c, float time, bool koi, bool bridge) {
    ellipse(t, c + sf::Vector2f(4.f, 6.f), 170.f, 56.f, alpha(cfg::INK, 40));
    ellipse(t, c, 168.f, 54.f, mix(cfg::SAND, cfg::INK_SOFT, 0.4f));              // stone rim
    ellipse(t, c, 154.f, 45.f, cfg::WATER);
    ellipse(t, c + sf::Vector2f(-30.f, -8.f), 100.f, 22.f, alpha(sf::Color::White, 70));
    for (int i = 0; i < 3; ++i) {                                               // ripples
        float ph = std::fmod(time * 0.35f + i * 0.33f, 1.f);
        ellipse(t, c + sf::Vector2f(-60.f + i * 55.f, 6.f), 14.f + ph * 40.f, 4.f + ph * 11.f, alpha(sf::Color::White, static_cast<std::uint8_t>(60 * (1.f - ph))));
    }
    ellipse(t, c + sf::Vector2f(-92.f, 10.f), 17.f, 6.f, cfg::SAGE_DARK);        // lily pads
    ellipse(t, c + sf::Vector2f(100.f, -14.f), 15.f, 5.f, cfg::SAGE_DARK);
    circle(t, c + sf::Vector2f(-92.f, 8.f), 4.f, cfg::SAKURA);
    if (koi) {
        for (int k = 0; k < 2; ++k) {
            float a = time * (0.5f + 0.12f * k) + k * 3.1f;
            sf::Vector2f p{c.x + std::cos(a) * 112.f, c.y + std::sin(a) * 28.f + 4.f};
            float rot = std::atan2(std::cos(a) * 28.f, -std::sin(a) * 112.f) * 180.f / PI;
            ellipse(t, p, 15.f, 6.f, k ? cfg::GOLD : cfg::SAKURA_DARK, rot);
            float rad = rot * PI / 180.f;
            ellipse(t, p - sf::Vector2f(std::cos(rad) * 15.f, std::sin(rad) * 15.f), 7.f, 4.f, alpha(k ? cfg::GOLD : cfg::SAKURA_DARK, 200), rot);
        }
    }
    if (bridge) {                                                              // small arched wooden bridge
        std::vector<sf::Vector2f> deck;
        for (int i = 0; i <= 14; ++i) { float x = -62.f + i * 124.f / 14.f; deck.push_back({c.x + x, c.y - 30.f - 26.f * std::sin(i / 14.f * PI)}); }
        for (int i = 14; i >= 0; --i) { float x = -62.f + i * 124.f / 14.f; deck.push_back({c.x + x, c.y - 20.f - 26.f * std::sin(i / 14.f * PI)}); }
        polygon(t, deck, cfg::WOOD);
        for (int i = 1; i < 14; i += 2) {
            float x = -62.f + i * 124.f / 14.f, y = c.y - 30.f - 26.f * std::sin(i / 14.f * PI);
            line(t, {c.x + x, y}, {c.x + x, y - 18.f}, 3.f, cfg::WOOD_DARK);
        }
        for (int i = 0; i < 14; ++i) {
            float x0 = -62.f + i * 124.f / 14.f, x1 = -62.f + (i + 1) * 124.f / 14.f;
            line(t, {c.x + x0, c.y - 48.f - 26.f * std::sin(i / 14.f * PI)}, {c.x + x1, c.y - 48.f - 26.f * std::sin((i + 1) / 14.f * PI)}, 3.f, cfg::WOOD_DARK);
        }
    }
}
} // namespace

int gardenStageFor(int completed) {
    int stage = 0;
    for (int i = 0; i <= GARDEN_MAX_STAGE; ++i) if (completed >= THRESHOLDS[i]) stage = i;
    return stage;
}
const char* gardenStageName(int stage) { return NAMES[stage < 0 ? 0 : (stage > GARDEN_MAX_STAGE ? GARDEN_MAX_STAGE : stage)]; }
int gardenThreshold(int stage) { return THRESHOLDS[stage < 0 ? 0 : (stage > GARDEN_MAX_STAGE ? GARDEN_MAX_STAGE : stage)]; }

void drawGarden(sf::RenderTarget& t, int stage, float time) {
    const float W = static_cast<float>(cfg::WIN_W), H = static_cast<float>(cfg::WIN_H);
    // sky and sun (circular motif)
    gradient(t, R(0, 0, W, HORIZON), cfg::IVORY, mix(cfg::SAGE_LIGHT, cfg::IVORY, 0.45f));
    circle(t, {905.f, 150.f}, 82.f, alpha(cfg::SAKURA, 90));
    circle(t, {905.f, 150.f}, 62.f, alpha(cfg::SAKURA_LIGHT, 230));
    ensoRing(t, {905.f, 150.f}, 104.f, 3.f, alpha(cfg::SAKURA, 90));
    // distant hills
    polygon(t, { {0.f, HORIZON}, {0.f, 330.f}, {140.f, 280.f}, {290.f, 340.f}, {430.f, 300.f}, {560.f, 360.f}, {700.f, 310.f}, {860.f, 350.f},
                 {1020.f, 290.f}, {1180.f, 335.f}, {W, HORIZON} }, alpha(cfg::SAGE, 120));
    polygon(t, { {0.f, HORIZON}, {0.f, 380.f}, {200.f, 345.f}, {380.f, 385.f}, {600.f, 355.f}, {820.f, 390.f}, {1000.f, 360.f}, {W, 392.f}, {W, HORIZON} },
            alpha(cfg::SAGE_DARK, 120));
    // ground
    gradient(t, R(0, HORIZON, W, H - HORIZON), mix(cfg::SAND, cfg::IVORY, 0.35f), mix(cfg::SAND, cfg::GOLD, 0.28f));
    rect(t, R(0, HORIZON - 2.f, W, 4.f), alpha(cfg::SAGE_DARK, 90));
    rakeLines(t);

    // ---- back layer ----
    if (stage >= 3) bamboo(t, time);
    if (stage >= 6) pine(t, {1085.f, HORIZON + 40.f}, time);
    if (stage >= 7) cherryTree(t, {585.f, HORIZON + 90.f}, time);
    if (stage >= 8) torii(t, {852.f, HORIZON + 24.f});
    // ---- middle layer ----
    if (stage >= 2) pond(t, {985.f, 585.f}, time, stage >= 5, stage >= 5);
    if (stage >= 4) lantern(t, {790.f, HORIZON + 100.f}, time);
    // ---- front layer ----
    stone(t, {520.f, 640.f}, 26.f, 14.f, mix(cfg::SAND, cfg::INK_SOFT, 0.45f));            // the first, lone stone
    ellipse(t, {520.f, 640.f}, 52.f, 20.f, alpha(cfg::GOLD, 50));
    if (stage >= 1) {
        const sf::Vector2f st[5] = { {640.f, 665.f}, {705.f, 632.f}, {765.f, 600.f}, {810.f, 560.f}, {838.f, 520.f} };
        for (int i = 0; i < 5; ++i) stone(t, st[i], 22.f - i * 1.5f, 11.f - i * 0.6f, mix(cfg::SAND, cfg::INK_SOFT, 0.32f));
    }
    if (stage >= 8) fallingPetals(t, time, 26, R(430.f, 40.f, 740.f, 640.f), 170);
    else if (stage >= 7) fallingPetals(t, time, 9, R(430.f, 40.f, 740.f, 640.f), 150);
    grain(t, R(0, 0, W, H), 160, 12, 11);
}
