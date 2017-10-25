#pragma once
#include <vector>

namespace harmful {
    enum wave_type {
        WAVE_SINE,
        WAVE_SAW
    };

    struct adsr_t {
        adsr_t() : attack_ms(0), decay_ms(0), release_ms(0), sustain_level(1.0f) {}

        int attack_ms, decay_ms, release_ms;
        float sustain_level;
    };

    struct harmonic_t {
        float freq_multiplier;
        float amplitude;
        wave_type type;
        adsr_t volume_envelope;
    };

    struct synth_t {
        std::vector<harmonic_t> harmonics;
        float volume;
    };

    void add_note(std::vector<float>& samples, float frequency, int start_ms, int duration_ms, float velocity, const synth_t& synth, int sample_rate);
}
