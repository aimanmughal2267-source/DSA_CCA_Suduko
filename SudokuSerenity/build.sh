#!/bin/bash
# Build without make. Run inside the MSYS2 MINGW64 terminal, in the project folder.
set -e
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude $(find src -name '*.cpp') -o build/SudokuSerenity.exe \
    -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
echo "Build OK  ->  build/SudokuSerenity.exe"
