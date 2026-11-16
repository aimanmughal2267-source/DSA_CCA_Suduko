#!/bin/bash
# Copies every MinGW64 DLL the game needs next to build/SudokuSerenity.exe so it also runs
# on a PC without MSYS2 (for example the evaluator's laptop).
set -e
cd build
ldd SudokuSerenity.exe | grep -i "/mingw64/" | awk '{print $3}' | while read -r dll; do
    cp -v "$dll" .
done
echo "Done. Zip the whole project folder (build/, assets/, data/, src/, include/) and hand it in."
