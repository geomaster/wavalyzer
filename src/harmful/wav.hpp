#pragma once
#include <vector>
#include <string>

namespace harmful {
    int write_wav(const std::string& filename, const std::vector<float>& samples, int sample_rate);
}
