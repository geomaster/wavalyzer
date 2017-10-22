#pragma once
#include <vector>
#include <complex>

namespace wavalyzer {
    typedef std::vector<float> fft_result_t;

    fft_result_t fft_from_samples(const std::vector<float>& samples,
                                  size_t sample_rate,
                                  size_t step_hertz,
                                  size_t min_hertz,
                                  size_t max_hertz,
                                  float window_normalization_factor = 1.0f);


}
