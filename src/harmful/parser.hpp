#pragma once
#include <string>
#include <vector>
#include <iostream>

namespace harmful {
    struct node_t {
        std::string key;
        std::vector<std::string> params;
        std::vector<node_t*> children;
    };

    node_t* parse_from(std::istream& s);
}
