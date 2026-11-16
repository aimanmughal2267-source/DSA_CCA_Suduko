#pragma once
#include <string>
#include "UIManager.h"

// The puzzle: wooden-framed board, number pad that adapts to the grid size,
// notes mode, undo/redo, guidance, pause and the completion animation.
class GameScreen : public Screen {
public:
    explicit GameScreen(AppContext& ctx);
    void onEnter() override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void onMouseDown(sf::Vector2f p) override;
    void onKey(sf::Keyboard::Key key, bool ctrl) override;

private:
    void layout();
    void select(int r, int c);
    void enterDigit(int digit);
    void say(const std::string& msg);
    void leaveToMenu();
    sf::FloatRect cellRect(int r, int c) const;
    sf::FloatRect padRect(int i) const;

    void drawBoard(sf::RenderTarget& t);
    void drawPanel(sf::RenderTarget& t);
    void drawCelebration(sf::RenderTarget& t);
    void drawGaveUp(sf::RenderTarget& t);

    float cell_ = 60.f, ox_ = 0.f, oy_ = 0.f;
    int   selR_ = -1, selC_ = -1;
    std::vector<bool> peers_;
    bool  recorded_ = false;
    float completeT_ = 0.f, pulse_ = 0.f, toastTimer_ = 0.f;
    std::string toast_;

    ui::Button undoBtn_, redoBtn_, eraseBtn_, notesBtn_, hintBtn_, pauseBtn_, saveBtn_, solutionBtn_, musicBtn_;
    ui::Button resumeBtn_, againBtn_, menuBtn_;
};
