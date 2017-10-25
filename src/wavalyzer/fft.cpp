#include "fft.hpp"
#include <iostream>

const float PI = 3.14159265359f;

using namespace wavalyzer;
using namespace std;

namespace wavalyzer {
    void fft(vector<complex<float>>& samples);
    size_t hertz_to_sample(int hertz, size_t sample_rate, size_t samples);
}

size_t wavalyzer::hertz_to_sample(int hertz, size_t sample_rate, size_t samples)
{
    return hertz * samples / sample_rate;
}

void wavalyzer::fft(vector<complex<float>>& samples)
{
    size_t n = samples.size();
    if (n <= 1) {
        return;
    }

    vector<complex<float>> even_samples, odd_samples;
    even_samples.reserve(n / 2);
    odd_samples.reserve(n / 2);

    for (size_t i = 0; i < n; i += 2) {
        even_samples.push_back(samples[i]);
        odd_samples.push_back(samples[i + 1]);
    }

    fft(even_samples);
    fft(odd_samples);

    for (size_t i = 0; i < n / 2; i++) {
        complex<float> T = polar(1.0f, -2.0f * PI * i / n) * odd_samples[i];
        samples[i] = even_samples[i] + T;
        samples[i + n / 2] = even_samples[i] - T;
    }
}

fft_result_t wavalyzer::fft_from_samples(const vector<float>& samples,
                                         size_t sample_rate,
                                         size_t step_hertz,
                                         size_t min_hertz,
                                         size_t max_hertz,
                                         float window_normalization_factor)
{
    vector<complex<float>> samples_c;
    samples_c.reserve(samples.size());

    for (float f : samples) {
        samples_c.push_back(f);
    }

    fft(samples_c);

    fft_result_t res;
    float factor = 1.0f * window_normalization_factor / samples.size();

    for (size_t hertz = min_hertz; hertz <= max_hertz; hertz += step_hertz) {
        int lower_hertz = static_cast<int>(hertz) - static_cast<int>(step_hertz);
        int upper_hertz = static_cast<int>(hertz) + static_cast<int>(step_hertz);

        int lower_sample = hertz_to_sample(lower_hertz, sample_rate, samples.size());
        int upper_sample = hertz_to_sample(upper_hertz, sample_rate, samples.size());

        float sum = 0.0f, mx = 0.0f;
        for (int sample = lower_sample; sample <= upper_sample; sample++) {
            sum += abs(samples_c[sample]);
            if (abs(samples_c[sample]) > mx) {
                mx = abs(samples_c[sample]);
            }
        }

        res.push_back(factor * sum / 2.0f);
    }

    return res;
}

