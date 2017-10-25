#include "common.hpp"
using namespace std;

int harmful::string_to_integer(const string& s)
{
    int pow10 = 1, res = 0;
    for (auto it = s.rbegin(); it != s.rend(); it++) {
        if (*it < '0' || *it > '9') {
            return -1;
        }

        res = (*it - '0') * pow10 + res;
        pow10 *= 10;
    }

    return res;
}

float harmful::note_to_frequency(const string& note)
{
    if (note.empty()) {
        return 0.0f;
    }

    if (note.length() > 3 && note[note.length() - 2] == 'H' && note[note.length() - 1] == 'z') {
        int parsed = string_to_integer(note.substr(0, note.length() - 2));
        if (parsed > 0) {
            return parsed;
        } else {
            return 0.0f;
        }
    }

    float frequency = 0.0f;
    bool sharp = (note.length() > 1 && note[1] == '#');

    switch(note[0]) {
        // Equal-tempered scale with A4=440Hz
        case 'C': frequency = sharp ? 17.32f : 16.35f; break;
        case 'D': frequency = sharp ? 19.45f : 18.35f; break;
        case 'E': frequency = sharp ? 0.0f   : 20.60f; break;
        case 'F': frequency = sharp ? 23.12f : 21.83f; break;
        case 'G': frequency = sharp ? 25.96f : 24.50f; break;
        case 'A': frequency = sharp ? 29.14f : 27.50f; break;
        case 'B': frequency = sharp ? 0.0f   : 30.87f; break;
        default: frequency = 0.0f;
    }

    if (frequency == 0.0f) {
        return 0.0f;
    }

    if (note.back() < '0' || note.back() > '9') {
        return 0.0f;
    }

    int octave = note.back() - '0';
    while (octave--) {
        frequency *= 2.0f;
    }

    return frequency;
}

bool harmful::ends_with(const string& s, const string& suffix)
{
    if (suffix.length() == 0) {
        return true;
    }

    if (s.length() < suffix.length()) {
        return false;
    }

    for (size_t ai = s.length() - suffix.length(), bi = 0; ai < s.length(); ai++, bi++) {
        if (s[ai] != suffix[bi]) {
            return false;
        }
    }

    return true;
}
