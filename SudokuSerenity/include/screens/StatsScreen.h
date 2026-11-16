#pragma once
#include "UIManager.h"

// "My Progress": totals, best times per grid size, results per difficulty, garden growth.
class StatsScreen : public Screen {
public:
    explicit StatsScreen(AppContext& ctx) : Screen(ctx) {}
    void draw(sf::RenderTarget& target) override;
    void onMouseDown(sf::Vector2f p) override;
    void onKey(sf::Keyboard::Key key, bool ctrl) override;
};
