#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include "parser.hpp"
#include "wav.hpp"
#include "sequencer.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <.harm file> <out .wav file>" << endl;
        return -1;
    }

    string fname = argv[1];
    ifstream f(fname);
    harmful::node_t* n = harmful::parse_from(f);
    if (n == nullptr) {
        cerr << "Parse error." << endl;
        return -1;
    }

    try {
        vector<float> samples = harmful::sequence(n, 44100);
        harmful::write_wav(string(argv[2]), samples, 44100);
    } catch (harmful::sequencer_exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }

    return 0;
}
