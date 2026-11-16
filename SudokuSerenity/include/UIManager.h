// ============================================================================
//  UIManager.h - owns the window, the shared AppContext and every screen.
//  OOP: abstract base class Screen (polymorphism) + derived screens.
// ============================================================================
#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <string>
#include "Audio.h"
#include "Draw.h"
#include "Game.h"
#include "History.h"
#include "Leaderboard.h"
#include "Player.h"
#include "ScoreManager.h"

enum class ScreenId { Home = 0, NewGame, Game, Result, Stats, Leaderboard, Profile, COUNT };

// What the results screen shows after a completed puzzle.
struct ResultInfo {
    bool valid = false;
    GameRecord rec;
    PersonalBest best;
    unsigned newAchievements = 0;
    int rank = 0, rankTotal = 0;                 // position among all registered players
    int gardenBefore = 0, gardenAfter = 0;
};

struct AppContext {
    AppContext();

    ui::Fonts      fonts;
    std::string    dir;                          // data folder
    PlayerManager  players;                      // hash table
    ScoreManager   scores;                       // BST
    History        history;                      // linked list
    GameSession    game;
    MusicPlayer    music;
    std::string    gameOwner;                    // player who owns the running game (for safe autosave)
    bool           relaxed = false;              // relaxation mode for the next game
    float          time = 0.f;
    ResultInfo     result;
    std::function<void(ScreenId)> goTo;
    std::function<void()>         quit;

    // ---- game flow helpers ----
    std::string  playerName() const;
    std::string  savePath() const;
    bool         hasSave() const;
    std::string  describeSave() const;           // "Advanced 9 x 9 - 12:30 played"
    void         startNew(Difficulty d);
    void         startDaily();
    bool         resume();
    void         autosave();                     // saves the running game (Continue)
    void         deleteSave();
    void         finishGame();                   // records history, scores, profile, achievements
    bool         dailyDoneToday();
    Difficulty   todaysDifficulty() const;
};

// Bottom navigation: 0 Home, 1 Statistics, 2 Leaderboard, 3 Profile.
inline void navigateTab(AppContext& ctx, int tab) {
    static const ScreenId ids[4] = { ScreenId::Home, ScreenId::Stats, ScreenId::Leaderboard, ScreenId::Profile };
    if (tab >= 0 && tab < 4) ctx.goTo(ids[tab]);
}

class Screen {
public:
    explicit Screen(AppContext& ctx) : ctx_(ctx) {}
    virtual ~Screen() = default;
    virtual void onEnter() {}
    virtual void update(float /*dt*/) {}
    virtual void draw(sf::RenderTarget& target) = 0;
    virtual void onMouseMove(sf::Vector2f p) { mouse_ = p; }
    virtual void onMouseDown(sf::Vector2f /*p*/) {}
    virtual void onKey(sf::Keyboard::Key /*key*/, bool /*ctrl*/) {}
    virtual void onText(char32_t /*ch*/) {}
    virtual void onWheel(float /*delta*/) {}
protected:
    bool hover(const ui::Button& b) const { return b.contains(mouse_); }
    AppContext&  ctx_;
    sf::Vector2f mouse_{0.f, 0.f};
};

class UIManager {
public:
    UIManager();
    int run();
private:
    int  runImpl();
    void goTo(ScreenId id);
    void applyLetterbox(sf::Vector2u size);

    AppContext                ctx_;
    sf::RenderWindow          window_;
    sf::View                  view_;
    std::unique_ptr<Screen>   screens_[static_cast<int>(ScreenId::COUNT)];
    Screen*                   current_ = nullptr;
    const char*               stage_ = "starting";
};
