# Sudoku Serenity - Japanese Calm and Aesthetic

A peaceful Sudoku game inspired by Japanese gardens. Built in **C++17** with **SFML 3** as a
**Data Structures and Algorithms** project. Complete puzzles and an empty sand garden slowly
grows into a beautiful Japanese garden.

## 1. Build and run (MSYS2 MINGW64 terminal)

```
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-sfml make
cd /c/path/to/SudokuSerenity
make
cd build
./SudokuSerenity.exe
```

* Needs **SFML 3.x** (check: `pacman -Qi mingw-w64-x86_64-sfml | grep Version`).
* If `pacman -S` says "target not found" but the package is already installed, ignore it.
* No `make`?  Use `./build.sh` or `mingw32-make`.
* No audio library on your machine?  Build with `make NOAUDIO=1` (the game then runs silently).
* To run on a PC **without MSYS2** (for example the evaluator's laptop): after `make`, run
  `./copy_dlls.sh`, then hand in the whole folder. Double-click `build/SudokuSerenity.exe`.
* To hide the black console window, remove the `#` before `LDFLAGS += -mwindows` in the
  Makefile, then `make clean && make`.

**Before submitting:** open `include/Config.h` and replace the placeholder course code, member
names and roll numbers. They appear under **About** on the home screen. The list may contain
1 to 6 members; the count is detected automatically.

### About "Chinese-looking text in files"
All source files, the Makefile and this README are plain **ASCII** (no special characters), so
they can never be mis-decoded. Garbled text that looks Chinese appears only when a text editor
opens a **binary** file such as `.o`, `.exe` or `.dll`. In this project all build output goes to
`build/`, never next to the sources. Do not open files inside `build/` in a text editor.

## 2. Gameplay

| Difficulty | Grid | Numbers | Sub-grids | Clues |
|---|---|---|---|---|
| Beginner (Easy)   | 4 x 4 | 1-4 | 2 x 2 | 8 of 16 |
| Focused (Medium)  | 6 x 6 | 1-6 | 2 x 3 | 16 of 36 |
| Advanced (Hard)   | 9 x 9 | 1-9 | 3 x 3 | 32 of 81 |
| Master (Expert)   | 9 x 9 | 1-9 | 3 x 3 | 24 of 81 |

The board, the number buttons and the notes layout adapt automatically to the grid size
(a 4 x 4 game shows only four number buttons). Every generated puzzle has **exactly one solution**.

Controls: click a square, then click a number or press `1`-`9` | `N` notes (pencil) |
`H` hint | `Ctrl+Z` or `U` undo | `Y` redo | `Del` erase | `P` pause | `Esc` save and menu.

Colours: sage = selected square, pale pink = incorrect entry, muted gold = same number and
completed rows, columns or boxes.

Features: timer, score, mistake counter, undo/redo, erase, notes mode, guidance (hints),
pause/resume, completion detection, **Relaxation mode** (no timer), **background music toggle**
(a calm melody synthesised in code, no audio files), **Continue** (resume the saved puzzle),
**Daily Challenge** (same puzzle for everyone on a given date), statistics per grid size and
difficulty, leaderboard, player profiles with avatars, **cherry blossom achievement badges**,
and the **garden** that grows with your completed puzzles:

| Completed puzzles | New garden element |
|---|---|
| 0 | Raked sand |
| 1 | Stepping stones |
| 2 | Koi pond |
| 3 | Bamboo grove |
| 5 | Stone lantern |
| 7 | Arched bridge and koi |
| 9 | Pine tree |
| 12 | Cherry blossom tree |
| 15 | Torii gate and falling petals |

Score = base (400 / 800 / 1500 / 2500) + time bonus (none in Relaxation mode)
- 40 per mistake - 60 per hint; Relaxation x0.9; Daily Challenge x1.15.

## 3. Data structures (all hand-written templates, no STL container used as the structure)

| Structure | File | Purpose |
|---|---|---|
| 2D array | `SudokuBoard` | 4x4, 6x6 or 9x9 board (+ given flags, pencil notes) |
| Stack | `ds/Stack.h`, `Game` | Undo and redo moves; DFS in the graph |
| Queue | `ds/Queue.h`, `Game` | Pending guidance (hint) requests; BFS in the graph |
| Linked list | `ds/LinkedList.h`, `History` | Game history (newest game first) |
| Hash table | `ds/HashTable.h`, `Player` | Player profiles, O(1) lookup by name |
| Binary search tree | `ds/BST.h`, `ScoreManager` | All scores ordered; rank queries in O(height) |
| Max-heap | `ds/MaxHeap.h`, `Leaderboard` | Top-player extraction |
| Graph | `ds/Graph.h`, `ConstraintGraph.cpp` | Each cell is a vertex; an edge joins cells sharing a row, column or sub-grid |

## 4. Algorithms

| Algorithm | Where |
|---|---|
| Recursion | puzzle generation and solving |
| Backtracking (MRV heuristic) | `SudokuSolver`, `PuzzleGenerator` |
| Validation (row, column, sub-grid) | `SudokuSolver::isValidPlacement`, graph conflict detection |
| Difficulty-based clue removal with uniqueness check | `PuzzleGenerator::carve` |
| Solution verification | `Game` (compares with the solution), `SudokuSolver::isSolved` |
| Searching | binary search (find a player), linear search (rank position), BST and hash lookups |
| Sorting | merge sort, quick sort (`Algorithms.h`) |

One generalised engine serves all sizes: the validation and solving code take a `GridSpec`
(size and sub-grid shape). Graph sizes: 4x4 = 16 vertices / degree 7, 6x6 = 36 / degree 12,
9x9 = 81 / degree 20.

## 5. OOP features

Classes and encapsulation (`GameSession`, `SudokuBoard`, `PlayerManager`, `ScoreManager`,
`History`, `UIManager`); abstract base class `Screen` with seven derived screens (polymorphism,
virtual functions); templates; RAII (containers free their nodes, `std::unique_ptr` owns screens);
deleted copy operations for node-owning containers; operator overloading (list iterator, `ScoreOrder`
functor); lambdas and `std::function`; nested classes; strongly typed enums; namespaces
(`cfg`, `ui`, `Algo`, `Solver`, `Generator`, `Scoring`, `util`); exception handling; composition
(`AppContext`).

## 6. Folder structure

```
SudokuSerenity/
  assets/  backgrounds/ icons/ fonts/ decorations/   (fonts: Playfair Display, Lato, Poppins - SIL OFL)
  include/ *.h, ds/*.h, screens/*.h
  src/     *.cpp, screens/*.cpp
  data/    players.txt scores.txt history.txt (+ save_<player>.txt for Continue)
  build/   SudokuSerenity.exe (created by make)
  Makefile build.sh copy_dlls.sh README.md
```

Typography: **Playfair Display** for titles, **Lato** for the interface, **Poppins** for Sudoku
numbers, all loaded from `assets/fonts/*.ttf` through `sf::Font`. If a file is missing the game
falls back to Georgia / Segoe UI from `C:\Windows\Fonts`. Font licences: `assets/fonts/OFL-*.txt`.

## 7. Troubleshooting

* "Fatal error ... Happened while: ..." - the game names the startup stage that failed.
* No sound: the toggle shows "No audio" when the PC has no usable audio device.
* DLL not found when double-clicking: run `./copy_dlls.sh`, or start from the MINGW64 terminal.
* Windows says the program must use the high-performance GPU: right-click the exe, choose
  "Run with graphics processor".
