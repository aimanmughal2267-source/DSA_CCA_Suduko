# ---------------------------------------------------------------------------
#  Sudoku Serenity - Makefile  (MSYS2 MINGW64 terminal:  make)
#  All build output goes to build/ so the source folders stay clean text.
#  No music library?  Build with:  make NOAUDIO=1
# ---------------------------------------------------------------------------
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
override CPPFLAGS += -Iinclude
override LDLIBS   += -lsfml-graphics -lsfml-window -lsfml-system

ifdef NOAUDIO
  override CPPFLAGS += -DSERENITY_NO_AUDIO
else
  override LDLIBS += -lsfml-audio
endif

# Uncomment to hide the console window in the final build:
# LDFLAGS += -mwindows

SRC := $(wildcard src/*.cpp src/screens/*.cpp)
OBJ := $(patsubst src/%.cpp,build/obj/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

ifeq ($(OS),Windows_NT)
  TARGET := build/SudokuSerenity.exe
else
  TARGET := build/SudokuSerenity
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)

build/obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	cd build && ./$(notdir $(TARGET))

clean:
	rm -rf build/obj $(TARGET)

-include $(DEP)
.PHONY: all clean run
