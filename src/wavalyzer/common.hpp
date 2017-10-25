#pragma once
#include <string>

namespace wavalyzer {
    float sample_level_to_dbfs(float sample_level, float noise_floor = -96.0f);
    std::string hertz_to_string(int hertz);

    template<typename T>
    T lerp(T a, T b, float alpha) {
        return alpha * a + (1.0f - alpha) * b;
    }
}
