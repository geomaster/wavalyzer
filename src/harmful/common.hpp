#pragma once
#include <string>

namespace harmful {
    int string_to_integer(const std::string& s);
    float note_to_frequency(const std::string& note);
    bool ends_with(const std::string& s, const std::string& suffix);
}
