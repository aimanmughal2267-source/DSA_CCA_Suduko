// ============================================================================
//  Draw.h - drawing helpers: fonts, rounded cards, buttons, icons, blossoms.
// ============================================================================
#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include "Config.h"

namespace ui {

enum class Align { Left, Center, Right };
enum class BtnStyle { Primary, Secondary, Accent, Ghost };
enum class Icon { Music, Leaf, Undo, Redo, Eraser, Pencil, Bulb, Pause, Play, Home, Chart, Trophy, User, Check, Back, Flag };

// Playfair Display (titles), Lato (interface), Poppins (Sudoku numbers).
struct Fonts {
    sf::Font title, ui, uiBold, number, numberBold;
    bool load();
};

struct Button {
    sf::FloatRect rect;
    std::string   label;
    bool          enabled = true;
    bool contains(sf::Vector2f p) const { return enabled && rect.contains(p); }
};

inline sf::Color alpha(sf::Color c, std::uint8_t a) { c.a = a; return c; }
sf::Color mix(sf::Color a, sf::Color b, float t);
inline sf::FloatRect R(float x, float y, float w, float h) { return sf::FloatRect({x, y}, {w, h}); }

// ---- shapes ----
void rect(sf::RenderTarget& t, const sf::FloatRect& r, sf::Color fill, sf::Color outline = sf::Color::Transparent,
          float thick = 0.f, const sf::RenderStates& st = sf::RenderStates::Default);
void roundRect(sf::RenderTarget& t, const sf::FloatRect& r, float radius, sf::Color fill,
               sf::Color outline = sf::Color::Transparent, float thick = 0.f,
               const sf::RenderStates& st = sf::RenderStates::Default);
void card(sf::RenderTarget& t, const sf::FloatRect& r, float radius = 18.f, sf::Color fill = cfg::PAPER,
          std::uint8_t shadow = 38);                                     // rounded card + delicate shadow
void circle(sf::RenderTarget& t, sf::Vector2f c, float radius, sf::Color fill,
            sf::Color outline = sf::Color::Transparent, float thick = 0.f,
            const sf::RenderStates& st = sf::RenderStates::Default);
void ellipse(sf::RenderTarget& t, sf::Vector2f c, float rx, float ry, sf::Color fill, float rotationDeg = 0.f,
             const sf::RenderStates& st = sf::RenderStates::Default);
void line(sf::RenderTarget& t, sf::Vector2f a, sf::Vector2f b, float thick, sf::Color color,
          const sf::RenderStates& st = sf::RenderStates::Default);
void gradient(sf::RenderTarget& t, const sf::FloatRect& r, sf::Color top, sf::Color bottom);
void polygon(sf::RenderTarget& t, const std::vector<sf::Vector2f>& pts, sf::Color fill,
             const sf::RenderStates& st = sf::RenderStates::Default);

// ---- text ----
float textWidth(const sf::Font& f, const std::string& s, unsigned size);
void  text(sf::RenderTarget& t, const sf::Font& f, const std::string& s, unsigned size, sf::Vector2f pos,
           sf::Color color, Align a = Align::Left, const sf::RenderStates& st = sf::RenderStates::Default);
void  textIn(sf::RenderTarget& t, const sf::Font& f, const std::string& s, unsigned size, const sf::FloatRect& box,
             sf::Color color, const sf::RenderStates& st = sf::RenderStates::Default);
std::vector<std::string> wrap(const sf::Font& f, const std::string& s, unsigned size, float maxWidth);

// ---- widgets and decorations ----
void button(sf::RenderTarget& t, const Fonts& f, const Button& b, bool hover, BtnStyle style = BtnStyle::Secondary,
            unsigned size = 18);
void iconButton(sf::RenderTarget& t, const Fonts& f, const Button& b, bool hover, Icon icon, bool active = false);
void drawIcon(sf::RenderTarget& t, Icon icon, sf::Vector2f center, float size, sf::Color color);
void toggle(sf::RenderTarget& t, const sf::FloatRect& r, bool on);
void blossom(sf::RenderTarget& t, sf::Vector2f c, float radius, float rotationDeg, sf::Color petal,
             std::uint8_t a = 255);
void petal(sf::RenderTarget& t, sf::Vector2f c, float size, float rotationDeg, sf::Color color);
void fallingPetals(sf::RenderTarget& t, float time, int count, const sf::FloatRect& area, std::uint8_t a = 150);
void ensoRing(sf::RenderTarget& t, sf::Vector2f c, float radius, float thick, sf::Color color);   // circular motif
void avatar(sf::RenderTarget& t, const Fonts& f, sf::Vector2f c, float radius, int id, const std::string& name);
void badge(sf::RenderTarget& t, const Fonts& f, sf::Vector2f c, float radius, int index, bool unlocked, bool showLabel);
void grain(sf::RenderTarget& t, const sf::FloatRect& r, int count, std::uint8_t a, unsigned seed);   // paper texture
void backdrop(sf::RenderTarget& t, float time);                       // calm paper background for inner screens

// ---- bottom navigation (Home, Statistics, Leaderboard, Profile) ----
sf::FloatRect navRect(int index);
int  navHit(sf::Vector2f p);                                          // -1 when nothing hit
void navBar(sf::RenderTarget& t, const Fonts& f, int active, sf::Vector2f mouse);

} // namespace ui
