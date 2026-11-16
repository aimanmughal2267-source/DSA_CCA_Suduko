#include "UIManager.h"
#include <exception>
#include <filesystem>
#include <iostream>
#include "Achievements.h"
#include "Config.h"
#include "Garden.h"
#include "Util.h"
#include "ds/Graph.h"
#include "screens/GameScreen.h"
#include "screens/HomeScreen.h"
#include "screens/LeaderboardScreen.h"
#include "screens/NewGameScreen.h"
#include "screens/ProfileScreen.h"
#include "screens/ResultScreen.h"
#include "screens/StatsScreen.h"

// ============================== AppContext ===================================
AppContext::AppContext()
    : dir(util::dataDir()),
      players(dir + "players.txt"),
      scores(dir + "scores.txt"),
      history(dir + "history.txt") {}

std::string AppContext::playerName() const {
    const PlayerProfile* p = players.current();
    return p ? p->name : std::string();
}

std::string AppContext::savePath() const { return dir + "save_" + util::fileSafe(playerName()) + ".txt"; }

bool AppContext::hasSave() const {
    std::error_code ec;
    return !playerName().empty() && std::filesystem::exists(savePath(), ec);
}

std::string AppContext::describeSave() const {
    GameSession tmp;
    if (!hasSave() || !tmp.loadFrom(savePath())) return "No saved puzzle yet";
    std::string s = std::string(difficultyName(tmp.difficulty())) + " " + sizeLabel(sizeIndexOf(tmp.difficulty()));
    if (tmp.relaxed()) s += " - relaxed";
    else               s += " - " + util::formatTime(static_cast<int>(tmp.elapsedSeconds())) + " played";
    return s;
}

void AppContext::deleteSave() {
    std::error_code ec;
    if (!playerName().empty()) std::filesystem::remove(savePath(), ec);
}

void AppContext::startNew(Difficulty d) {
    deleteSave();
    game.newGame(d, relaxed);
    gameOwner = playerName();
    players.recordStart(d);
}

Difficulty AppContext::todaysDifficulty() const { return Generator::dailyDifficulty(util::todayNumber()); }

void AppContext::startDaily() {
    deleteSave();
    game.newDaily(util::todayNumber(), relaxed);
    gameOwner = playerName();
    players.recordStart(game.difficulty());
}

bool AppContext::resume() {
    if (!hasSave()) return false;
    if (!game.loadFrom(savePath())) return false;
    gameOwner = playerName();
    return true;
}

void AppContext::autosave() {
    if (game.active() && game.state() == GameState::Playing && !playerName().empty() && gameOwner == playerName())
        game.saveTo(savePath());
}

bool AppContext::dailyDoneToday() {
    PlayerProfile* p = players.current();
    return p && p->dailyLast == util::todayNumber();
}

void AppContext::finishGame() {
    result = ResultInfo();
    PlayerProfile* p = players.current();
    if (!p || !game.finished()) return;
    GameRecord rec = game.makeRecord(p->name);
    result.gardenBefore = gardenStageFor(p->completedTotal());
    rec.id = history.add(rec);                                   // linked list
    if (rec.completed) {
        ScoreEntry e;
        e.name = rec.player; e.score = rec.score; e.seconds = rec.seconds;
        e.id = rec.id; e.difficulty = rec.difficulty; e.relaxed = rec.relaxed;
        scores.add(e);                                           // BST
        result.best = players.recordFinish(rec, result.newAchievements);   // hash table profile
        std::vector<Ranking> board = Leaderboard::build(scores, players, -1);   // heap ranking
        result.rank = Leaderboard::positionOf(board, rec.player) + 1;
        result.rankTotal = static_cast<int>(board.size());
    }
    result.rec = rec;
    result.valid = true;
    p = players.current();
    result.gardenAfter = gardenStageFor(p ? p->completedTotal() : 0);
    deleteSave();
}

// ============================== UIManager ====================================
UIManager::UIManager()
    : window_(sf::VideoMode({cfg::WIN_W, cfg::WIN_H}), cfg::WINDOW_TITLE),
      view_(sf::FloatRect({0.f, 0.f}, {static_cast<float>(cfg::WIN_W), static_cast<float>(cfg::WIN_H)})) {
    window_.setFramerateLimit(60);
}

void UIManager::applyLetterbox(sf::Vector2u size) {
    if (size.x == 0 || size.y == 0) return;
    float win = static_cast<float>(size.x) / static_cast<float>(size.y);
    float want = static_cast<float>(cfg::WIN_W) / static_cast<float>(cfg::WIN_H);
    sf::FloatRect vp({0.f, 0.f}, {1.f, 1.f});
    if (win > want) { float w = want / win; vp = sf::FloatRect({(1.f - w) / 2.f, 0.f}, {w, 1.f}); }
    else            { float h = win / want; vp = sf::FloatRect({0.f, (1.f - h) / 2.f}, {1.f, h}); }
    view_.setViewport(vp);
    window_.setView(view_);
}

void UIManager::goTo(ScreenId id) {
    current_ = screens_[static_cast<int>(id)].get();
    current_->onEnter();
}

int UIManager::run() {
    try {
        return runImpl();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\nHappened while: " << stage_ << "\n";
        return 1;
    }
}

int UIManager::runImpl() {
    stage_ = "loading fonts";
    if (!ctx_.fonts.load()) {
        std::cerr << "ERROR: no usable font found. Put the .ttf files into assets/fonts/ next to the game.\n";
        return 1;
    }
    stage_ = "checking the constraint graphs";
    for (int n : {4, 6, 9})
        if (!SudokuGraph::forSize(n).isConnected()) { std::cerr << "ERROR: constraint graph is inconsistent.\n"; return 1; }

    stage_ = "loading saved data";
    ctx_.players.load();
    ctx_.scores.load();
    ctx_.history.load();

    stage_ = "starting music";
    ctx_.music.setOn(true);

    ctx_.goTo = [this](ScreenId id) { goTo(id); };
    ctx_.quit = [this]() { window_.close(); };

    stage_ = "creating screens";
    screens_[static_cast<int>(ScreenId::Home)]        = std::make_unique<HomeScreen>(ctx_);
    screens_[static_cast<int>(ScreenId::NewGame)]     = std::make_unique<NewGameScreen>(ctx_);
    screens_[static_cast<int>(ScreenId::Game)]        = std::make_unique<GameScreen>(ctx_);
    screens_[static_cast<int>(ScreenId::Result)]      = std::make_unique<ResultScreen>(ctx_);
    screens_[static_cast<int>(ScreenId::Stats)]       = std::make_unique<StatsScreen>(ctx_);
    screens_[static_cast<int>(ScreenId::Leaderboard)] = std::make_unique<LeaderboardScreen>(ctx_);
    screens_[static_cast<int>(ScreenId::Profile)]     = std::make_unique<ProfileScreen>(ctx_);
    goTo(ctx_.players.current() ? ScreenId::Home : ScreenId::Profile);   // first launch: create a player

    stage_ = "running";
    applyLetterbox(window_.getSize());
    sf::Clock clock;
    while (window_.isOpen()) {
        while (const std::optional<sf::Event> ev = window_.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                ctx_.autosave();
                window_.close();
            } else if (const auto* rs = ev->getIf<sf::Event::Resized>()) {
                applyLetterbox(rs->size);
            } else if (const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) current_->onMouseDown(window_.mapPixelToCoords(mb->position));
            } else if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                current_->onKey(kp->code, kp->control);
            } else if (const auto* te = ev->getIf<sf::Event::TextEntered>()) {
                current_->onText(te->unicode);
            } else if (const auto* ws = ev->getIf<sf::Event::MouseWheelScrolled>()) {
                current_->onWheel(ws->delta);
            }
            if (!window_.isOpen()) break;
        }
        if (!window_.isOpen()) break;

        float dt = clock.restart().asSeconds();
        ctx_.time += dt;
        current_->onMouseMove(window_.mapPixelToCoords(sf::Mouse::getPosition(window_)));
        current_->update(dt);
        window_.clear(cfg::IVORY);
        current_->draw(window_);
        window_.display();
    }
    return 0;
}
