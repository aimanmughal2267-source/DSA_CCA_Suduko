// ============================================================================
//  Config.h - global constants: window, palette, data folders, team info.
//  >>> Edit the "PROJECT INFORMATION" block with your group's details. <<<
// ============================================================================
#pragma once
#include <SFML/Graphics/Color.hpp>

namespace cfg {

// ----------------------------- PROJECT INFORMATION --------------------------
inline constexpr const char* COURSE_NAME = "Data Structures & Algorithms";
inline constexpr const char* COURSE_CODE = "CT-159";

struct Member { const char* name; const char* roll; };
// Add or remove lines freely (1 to 6 members). The count is detected automatically.
inline constexpr Member MEMBERS[] = {
    { "Hunza Zulfiqar", "Roll No: CT-25256" },
    { "Aiman Yousuf", "Roll No: CT-25255" },
    { "Ayisha Nayyer Khan", "Roll No: CT-25262" },
};
inline constexpr int NUM_MEMBERS = static_cast<int>(sizeof(MEMBERS) / sizeof(MEMBERS[0]));
// ----------------------------------------------------------------------------

inline constexpr unsigned    WIN_W = 1180;
inline constexpr unsigned    WIN_H = 720;
inline constexpr const char* WINDOW_TITLE = "Sudoku Serenity";

// ----------------------------- COLOUR PALETTE -------------------------------
inline const sf::Color IVORY (0xF7, 0xF4, 0xEC);   // Warm Ivory
inline const sf::Color SAGE  (0xA8, 0xB9, 0x9A);   // Sage Green
inline const sf::Color SAKURA(0xDD, 0xB6, 0xB0);   // Sakura Pink
inline const sf::Color INK   (0x34, 0x46, 0x3B);   // Forest Ink
inline const sf::Color SAND  (0xE6, 0xD8, 0xC5);   // Sand Beige
inline const sf::Color GOLD  (0xB8, 0x9A, 0x70);   // Muted Gold

// Soft shades derived from the palette
inline const sf::Color PAPER       (0xFC, 0xFA, 0xF5);
inline const sf::Color SAGE_LIGHT  (0xD6, 0xE0, 0xCB);
inline const sf::Color SAGE_DARK   (0x7C, 0x93, 0x78);
inline const sf::Color SAKURA_LIGHT(0xF3, 0xDF, 0xDB);
inline const sf::Color SAKURA_DARK (0xC4, 0x8E, 0x8B);
inline const sf::Color INK_SOFT    (0x6A, 0x7D, 0x70);
inline const sf::Color WOOD        (0xA9, 0x86, 0x5E);
inline const sf::Color WOOD_DARK   (0x7A, 0x5C, 0x3E);
inline const sf::Color WATER       (0xBD, 0xD1, 0xC9);

} // namespace cfg
