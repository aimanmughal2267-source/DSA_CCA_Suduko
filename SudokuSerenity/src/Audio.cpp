#include "Audio.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#ifndef SERENITY_NO_AUDIO
#include <SFML/Audio.hpp>
#include <optional>
#endif

namespace {
constexpr double PI = 3.14159265358979;

#ifndef SERENITY_NO_AUDIO
// Deterministic ambient loop: plucked pentatonic notes + slow pad + soft echo.
std::vector<std::int16_t> synthesize(unsigned rate, int seconds) {
    const std::size_t L = static_cast<std::size_t>(rate) * static_cast<std::size_t>(seconds);
    std::vector<float> mix(L, 0.f);
    const double scale[] = { 293.66, 329.63, 369.99, 440.00, 493.88, 587.33, 659.25, 739.99 };   // D major pentatonic

    unsigned rng = 20240607u;
    auto next = [&rng]() { rng = rng * 1664525u + 1013904223u; return static_cast<double>((rng >> 8) & 0xFFFFu) / 65535.0; };

    int idx = 3;
    const double beat = 0.8;
    for (double tBeat = 0.0; tBeat < seconds - 0.1; tBeat += beat) {
        if (next() < 0.28) continue;                                      // leave some silence
        idx += (next() < 0.5 ? -1 : 1) * (1 + (next() < 0.3 ? 1 : 0));
        if (idx < 0) idx = 1;
        if (idx > 7) idx = 6;
        double amp = 0.20 + 0.12 * next();
        int voices = (next() < 0.25 && idx >= 3) ? 2 : 1;                 // sometimes add a lower partner
        for (int v = 0; v < voices; ++v) {
            double f = scale[v == 0 ? idx : idx - 3] * (v == 0 ? 1.0 : 0.5);
            std::size_t start = static_cast<std::size_t>(tBeat * rate);
            std::size_t len = static_cast<std::size_t>(rate) * 3;
            for (std::size_t k = 0; k < len; ++k) {
                double t = static_cast<double>(k) / rate;
                double env = std::exp(-t * 2.2) * (1.0 - std::exp(-t * 400.0));
                double s = std::sin(2 * PI * f * t) + 0.35 * std::sin(4 * PI * f * t) * std::exp(-t * 3.0)
                         + 0.12 * std::sin(6 * PI * f * t) * std::exp(-t * 5.0);
                mix[(start + k) % L] += static_cast<float>(amp * (v ? 0.6 : 1.0) * env * s);
            }
        }
    }
    // slow pad (whole cycles per loop so the seam is silent)
    const double padF[] = { 146.83, 220.0, 293.66 };
    for (double f0 : padF) {
        double f = std::round(f0 * seconds) / seconds;
        for (std::size_t k = 0; k < L; ++k) {
            double t = static_cast<double>(k) / rate;
            double lfo = 0.65 + 0.35 * std::sin(2 * PI * 2.0 * t / seconds);
            mix[k] += static_cast<float>(0.045 * lfo * std::sin(2 * PI * f * t));
        }
    }
    // two circular echo passes
    for (int pass = 0; pass < 2; ++pass) {
        std::size_t d = static_cast<std::size_t>(rate * (pass == 0 ? 0.29 : 0.43));
        float g = pass == 0 ? 0.30f : 0.22f;
        std::vector<float> src = mix;
        for (std::size_t k = 0; k < L; ++k) mix[k] += g * src[(k + L - d) % L];
    }
    float peak = 0.001f;
    for (float v : mix) peak = std::max(peak, std::fabs(v));
    std::vector<std::int16_t> out(L);
    for (std::size_t k = 0; k < L; ++k) out[k] = static_cast<std::int16_t>(mix[k] / peak * 0.7f * 32767.f);
    return out;
}
#endif
} // namespace

struct MusicPlayer::Impl {
    bool ok = false;
    bool on = false;
#ifndef SERENITY_NO_AUDIO
    sf::SoundBuffer buffer;
    std::optional<sf::Sound> sound;
#endif
};

MusicPlayer::MusicPlayer() : impl_(new Impl) {
#ifndef SERENITY_NO_AUDIO
    try {
        const unsigned rate = 22050;
        std::vector<std::int16_t> samples = synthesize(rate, 32);
        if (impl_->buffer.loadFromSamples(samples.data(), samples.size(), 1, rate, { sf::SoundChannel::Mono })) {
            impl_->sound.emplace(impl_->buffer);
            impl_->sound->setLooping(true);
            impl_->sound->setVolume(40.f);
            impl_->ok = true;
        }
    } catch (...) {
        impl_->ok = false;
    }
#endif
}

MusicPlayer::~MusicPlayer() {
#ifndef SERENITY_NO_AUDIO
    try { if (impl_ && impl_->sound) impl_->sound->stop(); } catch (...) {}
#endif
}

bool MusicPlayer::available() const { return impl_->ok; }
bool MusicPlayer::isOn() const { return impl_->ok && impl_->on; }

void MusicPlayer::setOn(bool on) {
    impl_->on = on;
#ifndef SERENITY_NO_AUDIO
    try {
        if (!impl_->ok || !impl_->sound) return;
        if (on) impl_->sound->play(); else impl_->sound->pause();
    } catch (...) { impl_->ok = false; }
#endif
}
