#pragma once
#include <vector>

namespace wavalyzer {
    void apply_hamming_window(std::vector<float>& samples);
    void apply_hann_window(std::vector<float>& samples);
}
