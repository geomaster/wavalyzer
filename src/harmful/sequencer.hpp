#pragma once
#include <iostream>
#include <vector>
#include "parser.hpp"

namespace harmful {
    class sequencer_exception : public std::exception {
    private:
        std::string message;
    public:
        sequencer_exception(const std::string& _message) : message(_message) {}
        virtual const char* what() const throw() override { return message.c_str(); }
    };

    std::vector<float> sequence(node_t* root, int sample_rate);
}
