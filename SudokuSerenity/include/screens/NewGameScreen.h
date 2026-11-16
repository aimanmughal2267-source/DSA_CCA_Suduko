#pragma once
#include "UIManager.h"

// Choose a path: Beginner 4x4, Focused 6x6, Advanced 9x9 or Master 9x9.
class NewGameScreen : public Screen {
public:
    explicit NewGameScreen(AppContext& ctx);
    void draw(sf::RenderTarget& target) override;
    void onMouseDown(sf::Vector2f p) override;
    void onKey(sf::Keyboard::Key key, bool ctrl) override;
private:
    void start(int level);
    sf::FloatRect cardRect(int i) const { return ui::R(40.f + i * 281.f, 118.f, 266.f, 388.f); }
    ui::Button backBtn_;
    sf::FloatRect relaxRect_;
};
