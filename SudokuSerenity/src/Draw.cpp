#include "Draw.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <sstream>
#include "Achievements.h"

namespace ui {

namespace {
constexpr float PI = 3.14159265f;

bool tryLoad(sf::Font& font, const std::vector<std::string>& names) {
    static const char* dirs[] = { "assets/fonts/", "../assets/fonts/", "../../assets/fonts/",
        "C:/Windows/Fonts/", "/usr/share/fonts/truetype/dejavu/", "/usr/share/fonts/truetype/liberation/",
        "/Library/Fonts/", "/System/Library/Fonts/Supplemental/" };
    for (const std::string& n : names)
        for (const char* d : dirs) {
            std::string path = std::string(d) + n;
            std::error_code ec;
            if (std::filesystem::exists(path, ec) && font.openFromFile(path)) return true;
        }
    return false;
}
} // namespace

bool Fonts::load() {
    struct Slot { sf::Font* f; std::vector<std::string> names; };
    Slot slots[] = {
        { &title,      { "PlayfairDisplay-Regular.ttf", "georgia.ttf", "DejaVuSerif.ttf", "LiberationSerif-Regular.ttf" } },
        { &ui,         { "Lato-Regular.ttf", "segoeui.ttf", "arial.ttf", "DejaVuSans.ttf", "LiberationSans-Regular.ttf" } },
        { &uiBold,     { "Lato-Bold.ttf", "segoeuib.ttf", "arialbd.ttf", "DejaVuSans-Bold.ttf", "LiberationSans-Bold.ttf" } },
        { &number,     { "Poppins-Medium.ttf", "segoeui.ttf", "arial.ttf", "DejaVuSans.ttf", "LiberationSans-Regular.ttf" } },
        { &numberBold, { "Poppins-SemiBold.ttf", "segoeuib.ttf", "arialbd.ttf", "DejaVuSans-Bold.ttf", "LiberationSans-Bold.ttf" } },
    };
    bool ok[5];
    int first = -1;
    for (int i = 0; i < 5; ++i) { ok[i] = tryLoad(*slots[i].f, slots[i].names); if (ok[i] && first < 0) first = i; }
    if (first < 0) return false;
    for (int i = 0; i < 5; ++i) if (!ok[i]) *slots[i].f = *slots[first].f;
    return true;
}

sf::Color mix(sf::Color a, sf::Color b, float t) {
    t = std::max(0.f, std::min(1.f, t));
    auto m = [t](int x, int y) { return static_cast<std::uint8_t>(x + (y - x) * t); };
    return sf::Color(m(a.r, b.r), m(a.g, b.g), m(a.b, b.b), m(a.a, b.a));
}

// ------------------------------- shapes --------------------------------------
void rect(sf::RenderTarget& t, const sf::FloatRect& r, sf::Color fill, sf::Color outline, float thick, const sf::RenderStates& st) {
    sf::RectangleShape s(r.size);
    s.setPosition(r.position);
    s.setFillColor(fill);
    if (thick > 0.f) { s.setOutlineColor(outline); s.setOutlineThickness(thick); }
    t.draw(s, st);
}

void roundRect(sf::RenderTarget& t, const sf::FloatRect& r, float radius, sf::Color fill, sf::Color outline, float thick,
               const sf::RenderStates& st) {
    radius = std::max(0.f, std::min(radius, std::min(r.size.x, r.size.y) / 2.f));
    const int seg = 7;
    const float w = r.size.x, h = r.size.y;
    sf::ConvexShape s(static_cast<std::size_t>(4 * (seg + 1)));
    sf::Vector2f centers[4] = { {radius, radius}, {w - radius, radius}, {w - radius, h - radius}, {radius, h - radius} };
    float start[4] = { 180.f, 270.f, 0.f, 90.f };
    std::size_t k = 0;
    for (int c = 0; c < 4; ++c)
        for (int i = 0; i <= seg; ++i) {
            float a = (start[c] + 90.f * static_cast<float>(i) / seg) * PI / 180.f;
            s.setPoint(k++, {centers[c].x + radius * std::cos(a), centers[c].y + radius * std::sin(a)});
        }
    s.setPosition(r.position);
    s.setFillColor(fill);
    if (thick > 0.f) { s.setOutlineColor(outline); s.setOutlineThickness(thick); }
    t.draw(s, st);
}

void card(sf::RenderTarget& t, const sf::FloatRect& r, float radius, sf::Color fill, std::uint8_t shadow) {
    for (int i = 3; i >= 1; --i)                                   // layered, delicate shadow
        roundRect(t, R(r.position.x - i, r.position.y - i + 4, r.size.x + 2.f * i, r.size.y + 2.f * i), radius + i,
                  alpha(cfg::INK, static_cast<std::uint8_t>(shadow / (i + 1) / 2)));
    roundRect(t, r, radius, fill, alpha(cfg::SAND, 255), 1.f);
}

void circle(sf::RenderTarget& t, sf::Vector2f c, float radius, sf::Color fill, sf::Color outline, float thick, const sf::RenderStates& st) {
    sf::CircleShape s(radius, 56);
    s.setOrigin({radius, radius});
    s.setPosition(c);
    s.setFillColor(fill);
    if (thick > 0.f) { s.setOutlineColor(outline); s.setOutlineThickness(thick); }
    t.draw(s, st);
}

void ellipse(sf::RenderTarget& t, sf::Vector2f c, float rx, float ry, sf::Color fill, float rot, const sf::RenderStates& st) {
    sf::CircleShape s(1.f, 48);
    s.setOrigin({1.f, 1.f});
    s.setScale({rx, ry});
    s.setPosition(c);
    s.setRotation(sf::degrees(rot));
    s.setFillColor(fill);
    t.draw(s, st);
}

void line(sf::RenderTarget& t, sf::Vector2f a, sf::Vector2f b, float thick, sf::Color color, const sf::RenderStates& st) {
    sf::Vector2f d = b - a;
    float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 0.01f) return;
    sf::RectangleShape s({len, thick});
    s.setOrigin({0.f, thick / 2.f});
    s.setPosition(a);
    s.setRotation(sf::degrees(std::atan2(d.y, d.x) * 180.f / PI));
    s.setFillColor(color);
    t.draw(s, st);
}

void gradient(sf::RenderTarget& t, const sf::FloatRect& r, sf::Color top, sf::Color bottom) {
    float x0 = r.position.x, y0 = r.position.y, x1 = x0 + r.size.x, y1 = y0 + r.size.y;
    sf::Vertex v[6];
    sf::Vector2f p[4] = { {x0, y0}, {x1, y0}, {x1, y1}, {x0, y1} };
    sf::Color c[4] = { top, top, bottom, bottom };
    int idx[6] = { 0, 1, 2, 0, 2, 3 };
    for (int i = 0; i < 6; ++i) { v[i].position = p[idx[i]]; v[i].color = c[idx[i]]; v[i].texCoords = {0.f, 0.f}; }
    t.draw(v, 6, sf::PrimitiveType::Triangles);
}

void polygon(sf::RenderTarget& t, const std::vector<sf::Vector2f>& pts, sf::Color fill, const sf::RenderStates& st) {
    sf::ConvexShape s(pts.size());
    for (std::size_t i = 0; i < pts.size(); ++i) s.setPoint(i, pts[i]);
    s.setFillColor(fill);
    t.draw(s, st);
}

// -------------------------------- text ---------------------------------------
float textWidth(const sf::Font& f, const std::string& s, unsigned size) { return sf::Text(f, s, size).getLocalBounds().size.x; }

void text(sf::RenderTarget& t, const sf::Font& f, const std::string& s, unsigned size, sf::Vector2f pos, sf::Color color,
          Align a, const sf::RenderStates& st) {
    sf::Text tx(f, s, size);
    tx.setFillColor(color);
    sf::FloatRect b = tx.getLocalBounds();
    float x = pos.x - b.position.x;
    if (a == Align::Center) x = pos.x - b.size.x / 2.f - b.position.x;
    else if (a == Align::Right) x = pos.x - b.size.x - b.position.x;
    tx.setPosition({std::floor(x), std::floor(pos.y)});
    t.draw(tx, st);
}

void textIn(sf::RenderTarget& t, const sf::Font& f, const std::string& s, unsigned size, const sf::FloatRect& box,
            sf::Color color, const sf::RenderStates& st) {
    sf::Text tx(f, s, size);
    tx.setFillColor(color);
    sf::FloatRect b = tx.getLocalBounds();
    sf::FloatRect rb = sf::Text(f, "H", size).getLocalBounds();      // cap height keeps labels level
    float x = box.position.x + (box.size.x - b.size.x) / 2.f - b.position.x;
    float y = box.position.y + (box.size.y - rb.size.y) / 2.f - rb.position.y;
    tx.setPosition({std::floor(x), std::floor(y)});
    t.draw(tx, st);
}

std::vector<std::string> wrap(const sf::Font& f, const std::string& s, unsigned size, float maxWidth) {
    std::vector<std::string> lines;
    std::istringstream in(s);
    std::string word, cur;
    while (in >> word) {
        std::string trial = cur.empty() ? word : cur + " " + word;
        if (!cur.empty() && textWidth(f, trial, size) > maxWidth) { lines.push_back(cur); cur = word; }
        else cur = trial;
    }
    if (!cur.empty()) lines.push_back(cur);
    return lines;
}

// ------------------------------- widgets -------------------------------------
void button(sf::RenderTarget& t, const Fonts& f, const Button& b, bool hover, BtnStyle style, unsigned size) {
    sf::Color fill, border, txt = cfg::INK;
    switch (style) {
        case BtnStyle::Primary: fill = cfg::SAGE;   border = cfg::SAGE_DARK; break;
        case BtnStyle::Accent:  fill = cfg::SAKURA; border = cfg::SAKURA_DARK; break;
        case BtnStyle::Ghost:   fill = alpha(cfg::PAPER, 200); border = cfg::SAND; break;
        default:                fill = cfg::SAND;   border = cfg::GOLD; break;
    }
    if (!b.enabled) { fill = mix(fill, cfg::IVORY, 0.6f); border = mix(border, cfg::IVORY, 0.6f); txt = alpha(cfg::INK, 110); }
    else if (hover) fill = mix(fill, sf::Color::White, 0.28f);
    float rad = std::min(b.rect.size.y / 2.f, 22.f);
    roundRect(t, R(b.rect.position.x, b.rect.position.y + 2, b.rect.size.x, b.rect.size.y), rad, alpha(cfg::INK, 22));
    roundRect(t, b.rect, rad, fill, border, 1.5f);
    textIn(t, f.uiBold, b.label, size, b.rect, txt);
}

void iconButton(sf::RenderTarget& t, const Fonts& f, const Button& b, bool hover, Icon icon, bool active) {
    sf::Color fill = active ? cfg::SAGE : cfg::SAND;
    if (!b.enabled) fill = mix(fill, cfg::IVORY, 0.6f);
    else if (hover) fill = mix(fill, sf::Color::White, 0.28f);
    float rad = 16.f;
    roundRect(t, R(b.rect.position.x, b.rect.position.y + 2, b.rect.size.x, b.rect.size.y), rad, alpha(cfg::INK, 22));
    roundRect(t, b.rect, rad, fill, active ? cfg::SAGE_DARK : cfg::GOLD, 1.5f);
    sf::Color ic = b.enabled ? cfg::INK : alpha(cfg::INK, 100);
    float cx = b.rect.position.x + b.rect.size.x / 2.f, cy = b.rect.position.y + b.rect.size.y / 2.f;
    if (b.label.empty()) { drawIcon(t, icon, {cx, cy}, std::min(b.rect.size.x, b.rect.size.y) * 0.42f, ic); return; }
    drawIcon(t, icon, {cx, cy - 8.f}, 16.f, ic);
    text(t, f.ui, b.label, 12, {cx, b.rect.position.y + b.rect.size.y - 20.f}, ic, Align::Center);
}

void drawIcon(sf::RenderTarget& t, Icon icon, sf::Vector2f c, float s, sf::Color col) {
    float th = std::max(1.8f, s * 0.12f);
    switch (icon) {
        case Icon::Music:
            ellipse(t, {c.x - s * 0.25f, c.y + s * 0.32f}, s * 0.28f, s * 0.2f, col, -20.f);
            line(t, {c.x - s * 0.03f, c.y + s * 0.25f}, {c.x - s * 0.03f, c.y - s * 0.5f}, th, col);
            line(t, {c.x - s * 0.03f, c.y - s * 0.5f}, {c.x + s * 0.42f, c.y - s * 0.28f}, th * 1.4f, col);
            break;
        case Icon::Leaf:
            ellipse(t, c, s * 0.5f, s * 0.26f, col, -40.f);
            line(t, {c.x - s * 0.42f, c.y + s * 0.42f}, {c.x + s * 0.3f, c.y - s * 0.3f}, th * 0.7f, mix(col, cfg::IVORY, 0.7f));
            break;
        case Icon::Undo: case Icon::Redo: {
            float d = (icon == Icon::Undo) ? -1.f : 1.f;
            for (int i = 0; i < 8; ++i) {
                float a0 = (200.f + i * 18.f) * PI / 180.f, a1 = (200.f + (i + 1) * 18.f) * PI / 180.f;
                sf::Vector2f p0{c.x + d * (s * 0.42f * std::cos(a0)) * -1.f, c.y + s * 0.34f * std::sin(a0) + s * 0.25f};
                sf::Vector2f p1{c.x + d * (s * 0.42f * std::cos(a1)) * -1.f, c.y + s * 0.34f * std::sin(a1) + s * 0.25f};
                line(t, p0, p1, th, col);
            }
            polygon(t, { {c.x + d * s * 0.5f, c.y - s * 0.12f}, {c.x + d * s * 0.5f - d * s * 0.34f, c.y - s * 0.34f},
                         {c.x + d * s * 0.5f - d * s * 0.34f, c.y + s * 0.1f} }, col);
            break;
        }
        case Icon::Eraser: {
            sf::Transform tr; tr.translate(c); tr.rotate(sf::degrees(-35.f));
            roundRect(t, R(-s * 0.5f, -s * 0.24f, s, s * 0.48f), 4.f, col, sf::Color::Transparent, 0.f, sf::RenderStates(tr));
            rect(t, R(-s * 0.5f, -s * 0.24f, s * 0.36f, s * 0.48f), mix(col, cfg::IVORY, 0.55f), sf::Color::Transparent, 0.f, sf::RenderStates(tr));
            break;
        }
        case Icon::Pencil: {
            sf::Transform tr; tr.translate(c); tr.rotate(sf::degrees(-45.f));
            sf::RenderStates st(tr);
            rect(t, R(-s * 0.2f, -s * 0.12f, s * 0.7f, s * 0.24f), col, sf::Color::Transparent, 0.f, st);
            polygon(t, { {-s * 0.2f, -s * 0.12f}, {-s * 0.55f, 0.f}, {-s * 0.2f, s * 0.12f} }, col, st);
            break;
        }
        case Icon::Bulb:
            circle(t, {c.x, c.y - s * 0.1f}, s * 0.34f, col);
            rect(t, R(c.x - s * 0.17f, c.y + s * 0.2f, s * 0.34f, s * 0.16f), col);
            rect(t, R(c.x - s * 0.11f, c.y + s * 0.4f, s * 0.22f, s * 0.08f), col);
            break;
        case Icon::Pause:
            roundRect(t, R(c.x - s * 0.34f, c.y - s * 0.4f, s * 0.24f, s * 0.8f), 2.f, col);
            roundRect(t, R(c.x + s * 0.1f, c.y - s * 0.4f, s * 0.24f, s * 0.8f), 2.f, col);
            break;
        case Icon::Play:
            polygon(t, { {c.x - s * 0.28f, c.y - s * 0.42f}, {c.x + s * 0.42f, c.y}, {c.x - s * 0.28f, c.y + s * 0.42f} }, col);
            break;
        case Icon::Home:
            polygon(t, { {c.x - s * 0.5f, c.y - s * 0.02f}, {c.x, c.y - s * 0.5f}, {c.x + s * 0.5f, c.y - s * 0.02f} }, col);
            rect(t, R(c.x - s * 0.34f, c.y - s * 0.04f, s * 0.68f, s * 0.46f), col);
            rect(t, R(c.x - s * 0.1f, c.y + s * 0.14f, s * 0.2f, s * 0.28f), cfg::IVORY);
            break;
        case Icon::Chart:
            rect(t, R(c.x - s * 0.42f, c.y + s * 0.05f, s * 0.2f, s * 0.37f), col);
            rect(t, R(c.x - s * 0.1f, c.y - s * 0.2f, s * 0.2f, s * 0.62f), col);
            rect(t, R(c.x + s * 0.22f, c.y - s * 0.44f, s * 0.2f, s * 0.86f), col);
            break;
        case Icon::Trophy:
            polygon(t, { {c.x - s * 0.34f, c.y - s * 0.42f}, {c.x + s * 0.34f, c.y - s * 0.42f}, {c.x + s * 0.2f, c.y + s * 0.1f},
                         {c.x - s * 0.2f, c.y + s * 0.1f} }, col);
            rect(t, R(c.x - s * 0.06f, c.y + s * 0.1f, s * 0.12f, s * 0.22f), col);
            rect(t, R(c.x - s * 0.26f, c.y + s * 0.32f, s * 0.52f, s * 0.1f), col);
            break;
        case Icon::User:
            circle(t, {c.x, c.y - s * 0.22f}, s * 0.22f, col);
            ellipse(t, {c.x, c.y + s * 0.42f}, s * 0.42f, s * 0.42f, col);
            break;
        case Icon::Check:
            line(t, {c.x - s * 0.38f, c.y + s * 0.02f}, {c.x - s * 0.1f, c.y + s * 0.32f}, th * 1.3f, col);
            line(t, {c.x - s * 0.1f, c.y + s * 0.32f}, {c.x + s * 0.42f, c.y - s * 0.3f}, th * 1.3f, col);
            break;
        case Icon::Back:
            line(t, {c.x + s * 0.2f, c.y - s * 0.36f}, {c.x - s * 0.22f, c.y}, th * 1.3f, col);
            line(t, {c.x - s * 0.22f, c.y}, {c.x + s * 0.2f, c.y + s * 0.36f}, th * 1.3f, col);
            break;
        case Icon::Flag:
            line(t, {c.x - s * 0.3f, c.y - s * 0.45f}, {c.x - s * 0.3f, c.y + s * 0.45f}, th, col);
            polygon(t, { {c.x - s * 0.3f, c.y - s * 0.45f}, {c.x + s * 0.4f, c.y - s * 0.22f}, {c.x - s * 0.3f, c.y + s * 0.02f} }, col);
            break;
    }
}

void toggle(sf::RenderTarget& t, const sf::FloatRect& r, bool on) {
    roundRect(t, r, r.size.y / 2.f, on ? cfg::SAGE : cfg::SAND, on ? cfg::SAGE_DARK : cfg::GOLD, 1.5f);
    float kx = on ? r.position.x + r.size.x - r.size.y / 2.f : r.position.x + r.size.y / 2.f;
    circle(t, {kx + 1.f, r.position.y + r.size.y / 2.f + 1.5f}, r.size.y / 2.f - 4.f, alpha(cfg::INK, 40));
    circle(t, {kx, r.position.y + r.size.y / 2.f}, r.size.y / 2.f - 4.f, cfg::PAPER);
}

void petal(sf::RenderTarget& t, sf::Vector2f c, float size, float rot, sf::Color color) {
    ellipse(t, c, size * 0.5f, size * 0.3f, color, rot);
    ellipse(t, c + sf::Vector2f(size * 0.1f * std::cos(rot * PI / 180.f), size * 0.1f * std::sin(rot * PI / 180.f)),
            size * 0.3f, size * 0.12f, mix(color, sf::Color::White, 0.35f), rot);
}

void blossom(sf::RenderTarget& t, sf::Vector2f c, float radius, float rot, sf::Color col, std::uint8_t a) {
    sf::Color pc = alpha(col, a);
    for (int i = 0; i < 5; ++i) {
        float ang = rot + i * 72.f;
        float rad = ang * PI / 180.f;
        sf::Vector2f pos{c.x + std::cos(rad) * radius * 0.52f, c.y + std::sin(rad) * radius * 0.52f};
        ellipse(t, pos, radius * 0.5f, radius * 0.36f, pc, ang);
        ellipse(t, pos + sf::Vector2f(std::cos(rad) * radius * 0.2f, std::sin(rad) * radius * 0.2f),
                radius * 0.14f, radius * 0.1f, alpha(mix(col, cfg::IVORY, 0.55f), a), ang);
    }
    circle(t, c, radius * 0.16f, alpha(cfg::GOLD, a));
}

void fallingPetals(sf::RenderTarget& t, float time, int count, const sf::FloatRect& area, std::uint8_t a) {
    for (int i = 0; i < count; ++i) {
        float seed = static_cast<float>(i) * 1.618f;
        float speed = 22.f + std::fmod(seed * 13.f, 26.f);
        float cycle = area.size.y + 60.f;
        float y = std::fmod(time * speed + seed * 97.f, cycle) - 30.f;
        float x = area.position.x + std::fmod(seed * 211.f, area.size.x) + std::sin(time * 0.9f + seed) * 26.f;
        float size = 9.f + std::fmod(seed * 7.f, 8.f);
        petal(t, {x, area.position.y + y}, size, time * 40.f * (0.5f + std::fmod(seed, 1.f)) + seed * 60.f,
              alpha(i % 3 == 0 ? cfg::SAKURA_LIGHT : cfg::SAKURA, a));
    }
}

void ensoRing(sf::RenderTarget& t, sf::Vector2f c, float radius, float thick, sf::Color color) {
    sf::CircleShape s(radius, 90);
    s.setOrigin({radius, radius});
    s.setPosition(c);
    s.setFillColor(sf::Color::Transparent);
    s.setOutlineColor(color);
    s.setOutlineThickness(-thick);
    t.draw(s);
}

void avatar(sf::RenderTarget& t, const Fonts& f, sf::Vector2f c, float r, int id, const std::string& name) {
    static const sf::Color cols[6] = { cfg::SAKURA, cfg::SAGE, cfg::GOLD, cfg::WATER, cfg::SAKURA_DARK, cfg::SAGE_DARK };
    id = ((id % 6) + 6) % 6;
    circle(t, c + sf::Vector2f(0.f, 2.f), r, alpha(cfg::INK, 30));
    circle(t, c, r, cols[id], cfg::PAPER, 2.f);
    ensoRing(t, c, r * 0.8f, 1.5f, alpha(cfg::PAPER, 190));
    if (id % 2 == 0) blossom(t, c + sf::Vector2f(r * 0.55f, -r * 0.55f), r * 0.22f, 10.f, cfg::PAPER, 220);
    std::string ch = name.empty() ? "?" : std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(name[0]))));
    textIn(t, f.title, ch, static_cast<unsigned>(r * 1.05f), R(c.x - r, c.y - r, 2 * r, 2 * r), cfg::INK);
}

void badge(sf::RenderTarget& t, const Fonts& f, sf::Vector2f c, float r, int index, bool unlocked, bool showLabel) {
    circle(t, c + sf::Vector2f(0.f, 2.f), r, alpha(cfg::INK, 26));
    circle(t, c, r, unlocked ? cfg::SAKURA_LIGHT : mix(cfg::SAND, cfg::IVORY, 0.4f), unlocked ? cfg::SAKURA : cfg::SAND, 2.f);
    if (unlocked) blossom(t, c, r * 0.7f, static_cast<float>(index) * 13.f, cfg::SAKURA);
    else          blossom(t, c, r * 0.7f, static_cast<float>(index) * 13.f, mix(cfg::SAND, cfg::GOLD, 0.25f), 150);
    if (showLabel)
        text(t, f.ui, achievementDef(index).name, 12, {c.x, c.y + r + 6.f}, unlocked ? cfg::INK : alpha(cfg::INK, 120), Align::Center);
}

void grain(sf::RenderTarget& t, const sf::FloatRect& r, int count, std::uint8_t a, unsigned seed) {
    sf::VertexArray va(sf::PrimitiveType::Triangles, static_cast<std::size_t>(count) * 6);
    unsigned s = seed * 2654435761u + 12345u;
    auto next = [&s]() { s = s * 1664525u + 1013904223u; return static_cast<float>((s >> 8) & 0xFFFFu) / 65535.f; };
    for (int i = 0; i < count; ++i) {
        float x = r.position.x + next() * r.size.x, y = r.position.y + next() * r.size.y;
        float w = 1.f + next() * 3.f, h = 1.f;
        sf::Color col = alpha(cfg::GOLD, static_cast<std::uint8_t>(a * (0.4f + 0.6f * next())));
        sf::Vector2f p[4] = { {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h} };
        int idx[6] = { 0, 1, 2, 0, 2, 3 };
        for (int k = 0; k < 6; ++k) { va[static_cast<std::size_t>(i) * 6 + k].position = p[idx[k]]; va[static_cast<std::size_t>(i) * 6 + k].color = col; }
    }
    t.draw(va);
}

void backdrop(sf::RenderTarget& t, float time) {
    gradient(t, R(0, 0, cfg::WIN_W, cfg::WIN_H), cfg::IVORY, mix(cfg::IVORY, cfg::SAND, 0.75f));
    ensoRing(t, {1120.f, 90.f}, 150.f, 5.f, alpha(cfg::SAGE, 60));
    ensoRing(t, {1120.f, 90.f}, 120.f, 2.f, alpha(cfg::SAGE, 45));
    ensoRing(t, {60.f, 660.f}, 130.f, 5.f, alpha(cfg::SAKURA, 60));
    ensoRing(t, {60.f, 660.f}, 100.f, 2.f, alpha(cfg::SAKURA, 45));
    grain(t, R(0, 0, cfg::WIN_W, cfg::WIN_H), 260, 16, 7);
    fallingPetals(t, time, 7, R(0, 0, cfg::WIN_W, cfg::WIN_H), 70);
}

// --------------------------------- navigation --------------------------------
sf::FloatRect navRect(int index) { return R(292.f + index * 150.f, 654.f, 142.f, 46.f); }

int navHit(sf::Vector2f p) {
    for (int i = 0; i < 4; ++i) if (navRect(i).contains(p)) return i;
    return -1;
}

void navBar(sf::RenderTarget& t, const Fonts& f, int active, sf::Vector2f mouse) {
    const char* labels[4] = { "Home", "Statistics", "Leaderboard", "Profile" };
    Icon icons[4] = { Icon::Home, Icon::Chart, Icon::Trophy, Icon::User };
    card(t, R(282.f, 646.f, 614.f, 62.f), 31.f, alpha(cfg::PAPER, 240), 30);
    for (int i = 0; i < 4; ++i) {
        sf::FloatRect r = navRect(i);
        bool on = (i == active), h = r.contains(mouse);
        if (on) roundRect(t, r, 23.f, cfg::SAGE_LIGHT, cfg::SAGE, 1.5f);
        else if (h) roundRect(t, r, 23.f, alpha(cfg::SAND, 150));
        sf::Color col = on ? cfg::INK : cfg::INK_SOFT;
        drawIcon(t, icons[i], {r.position.x + 26.f, r.position.y + 23.f}, 17.f, col);
        text(t, on ? f.uiBold : f.ui, labels[i], 16, {r.position.x + 44.f, r.position.y + 12.f}, col);
    }
}

} // namespace ui
