#include "common.hpp"
#include <cmath>

using namespace wavalyzer;
using namespace std;

float wavalyzer::sample_level_to_dbfs(float sample_level, float noise_floor)
{
    float dbfs = 20.0f * log(sample_level);
    if (dbfs < noise_floor) {
        return noise_floor;
    }

    return dbfs;
}

string wavalyzer::hertz_to_string(int hertz)
{
    if (hertz < 1000) {
        return to_string(hertz) + "Hz";
    } else {
        return to_string(hertz / 1000) + "."  + to_string(hertz % 1000 / 100) + "kHz";
    }
}

