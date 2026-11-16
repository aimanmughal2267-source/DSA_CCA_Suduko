#pragma once
#include "UIManager.h"

// "Beautifully Solved!" - completion summary, personal bests, badges, garden growth.
class ResultScreen : public Screen {
public:
    explicit ResultScreen(AppContext& ctx);
    void onEnter() override { enterTime_ = ctx_.time; }
    void draw(sf::RenderTarget& target) override;
    void onMouseDown(sf::Vector2f p) override;
    void onKey(sf::Keyboard::Key key, bool ctrl) override;
private:
    void nextPuzzle();
    ui::Button nextBtn_, menuBtn_;
    float enterTime_ = 0.f;
};
