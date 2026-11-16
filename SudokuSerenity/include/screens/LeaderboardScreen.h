#pragma once
#include <string>
#include <vector>
#include "UIManager.h"

// Rankings of all registered players. Heap extraction + BST statistics + binary search.
class LeaderboardScreen : public Screen {
public:
    explicit LeaderboardScreen(AppContext& ctx) : Screen(ctx) {}
    void onEnter() override;
    void draw(sf::RenderTarget& target) override;
    void onMouseDown(sf::Vector2f p) override;
    void onKey(sf::Keyboard::Key key, bool ctrl) override;
    void onText(char32_t ch) override;
private:
    void refresh();
    void search();
    sf::FloatRect tabRect(int i) const { return ui::R(40.f + i * 150.f, 96.f, 140.f, 40.f); }
    int tab_ = -1;                       // -1 all sizes, else 0..2
    std::vector<Ranking> rows_;
    std::string query_, found_;
    bool searched_ = false, typing_ = false;
};
