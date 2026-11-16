// ============================================================================
//  SUDOKU SERENITY - a calm Japanese-garden Sudoku (C++17, SFML 3)
//  Data Structures & Algorithms project.
// ============================================================================
#include <SFML/GpuPreference.hpp>
#include <exception>
#include <iostream>
#include "UIManager.h"

// On laptops with two graphics chips ask Windows for the faster one (no effect elsewhere).
SFML_DEFINE_DISCRETE_GPU_PREFERENCE

int main() {
    const char* stage = "creating window";
    try {
        UIManager app;                 // opens the window
        stage = "running";
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\nHappened while: " << stage << "\n";
        return 1;
    }
}
