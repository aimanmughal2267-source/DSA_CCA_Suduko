#pragma once
#include <string>
#include <vector>
#include "UIManager.h"

// Players: add, switch and view profiles; per-size progress; cherry blossom badges.
class ProfileScreen : public Screen {
public:
    explicit ProfileScreen(AppContext& ctx);
    void onEnter() override;
    void draw(sf::RenderTarget& target) override;
    void onMouseDown(sf::Vector2f p) override;
    void onKey(sf::Keyboard::Key key, bool ctrl) override;
    void onText(char32_t ch) override;
    void onWheel(float delta) override;
private:
    void addPlayer();
    sf::FloatRect rowRect(int i) const { return ui::R(60.f, 152.f + i * 56.f, 390.f, 50.f); }
    sf::FloatRect avatarRect(int i) const { return ui::R(60.f + i * 62.f, 540.f, 52.f, 52.f); }
    std::vector<std::string> names_;
    std::string input_, message_;
    int scroll_ = 0, avatar_ = 0;
    bool typing_ = true;
    ui::Button addBtn_;
    static constexpr int ROWS = 4;
};
