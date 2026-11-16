// ============================================================================
//  Garden.h - the Japanese garden that grows as puzzles are completed.
//  Stage 0 is an empty raked-sand garden; each stage adds a new element.
// ============================================================================
#pragma once
#include <SFML/Graphics.hpp>

inline constexpr int GARDEN_MAX_STAGE = 8;

int         gardenStageFor(int completedPuzzles);
const char* gardenStageName(int stage);            // the element that appears at this stage
int         gardenThreshold(int stage);            // puzzles needed to reach this stage
void        drawGarden(sf::RenderTarget& target, int stage, float time);   // full-window scenery
