#pragma once
#include <string>
#include "UIManager.h"

// Japanese garden scenery with the main menu cards. The garden grows with every win.
class HomeScreen : public Screen {
public:
    explicit HomeScreen(AppContext& ctx);
    void onEnter() override;
    void draw(sf::RenderTarget& target) override;
    void onMouseDown(sf::Vector2f p) override;
private:
    void activate(int card);
    void pill(sf::RenderTarget& t, const ui::Button& b, ui::Icon icon, bool on);
    ui::Button cards_[4];
    ui::Button pillPlayer_, pillMusic_, pillRelax_, pillAbout_;
    std::string saveInfo_;
    bool showAbout_ = false;
};
