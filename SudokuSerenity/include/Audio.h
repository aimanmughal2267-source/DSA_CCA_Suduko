// ============================================================================
//  Audio.h - calm background music, SYNTHESISED at start-up (no audio files).
//  A soft pentatonic koto-like melody over a slow pad, looped forever.
//  If no audio device exists the game simply runs without music.
// ============================================================================
#pragma once
#include <memory>

class MusicPlayer {
public:
    MusicPlayer();
    ~MusicPlayer();
    bool available() const;          // false if audio is unsupported on this PC
    bool isOn() const;
    void setOn(bool on);
    void toggle() { setOn(!isOn()); }
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
