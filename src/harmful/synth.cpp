#include "synth.hpp"
#include <iostream>
#include <numeric>
#include <cmath>

using namespace harmful;
using namespace std;

namespace harmful {
    float get_envelope_level(const adsr_t& envelope, float ms, float duration_ms);
    void add_harmonic(vector<float>& samples, float frequency, float amplitude, float first_ms, float last_ms, const harmonic_t& harmonic, int sample_rate);
    void clip(float& sample) {
        if (sample > 1.0f) {
            sample = 1.0f;
        }
        if (sample < -1.0f) {
            sample = -1.0f;
        }
    }
}

float harmful::get_envelope_level(const adsr_t& envelope, float ms, float duration_ms)
{
    // Attack phase
    if (ms < envelope.attack_ms) {
        return ms / envelope.attack_ms;
    }

    // Decay phase
    if (ms < envelope.decay_ms + envelope.attack_ms) {
        return envelope.sustain_level + (1.0f - envelope.sustain_level) * (1.0f - (ms - envelope.attack_ms) / envelope.decay_ms);
    }

    // Sustain phase
    if (ms < duration_ms) {
        return envelope.sustain_level;
    }

    // Release phase
    return max(envelope.sustain_level * (1.0f - (ms - duration_ms) / envelope.release_ms), 0.0f);
}

void harmful::add_harmonic(vector<float>& samples,
                           float frequency,
                           float amplitude,
                           float first_ms,
                           float last_ms,
                           const harmonic_t& harmonic,
                           int sample_rate)

{
    float samples_per_ms = static_cast<float>(sample_rate) / 1000.0f,
          delta = 1.0f / sample_rate;

    int first_sample = first_ms * sample_rate / 1000.0f;
    int last_sample = last_ms * sample_rate / 1000.0f;
    int rel_samples = harmonic.volume_envelope.release_ms * sample_rate / 1000.0f;

    float time = 0.0f;
    for (int i = first_sample; i <= (last_sample + rel_samples) && i < samples.size(); i++, time += delta) {
        float curr_ms = i * 1000.0f / sample_rate;

        float f = harmonic.freq_multiplier *
                  frequency,

              A = harmonic.amplitude *
                  amplitude *
                  get_envelope_level(harmonic.volume_envelope, curr_ms - first_ms, last_ms - first_ms),

              function = harmonic.type == WAVE_SINE ?
                            sin(2 * M_PI * f * time) :
                            2.0f * (time * f - floor(0.5f + time * f)),

              level = A * function;

        samples[i] += level;
        clip(samples[i]);
    }
}

void harmful::add_note(std::vector<float>& samples,
                       float frequency,
                       int start_ms,
                       int duration_ms,
                       float velocity,
                       const synth_t& synth,
                       int sample_rate)
{
    float amplitude = velocity * synth.volume;
    for (const auto& harmonic : synth.harmonics) {
        add_harmonic(samples, frequency, amplitude, start_ms, start_ms + duration_ms - 1, harmonic, sample_rate);
    }
}

