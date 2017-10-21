#include <iostream>
#include <fstream>
#include <vector>
#include "wav.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <wavfile>" << endl;
        return -1;
    }

    try {
        ifstream f(argv[1]);
        wavalyzer::wav_file w(f);

        cout << "Channels: " << w.get_channels() << endl;
        cout << "Total samples: " << w.get_total_samples() << endl;
        cout << "Sample rate: " << w.get_sample_rate() << endl;

        vector<float> samples;
        w.read_samples(samples, 400);

        for (int i = -15; i <= 15; i++) {
            float lower = (float)i / 15.0f - 0.5f / 15.0f, upper = (float)i / 15.0f + 0.5f / 15.0f;

            for (int j = 0; j < samples.size() / 2; j++) {
                if (samples[2*j] >= lower && samples[2*j] <= upper) {
                    cout << ":";
                } else {
                    cout << " ";
                }
            }

            cout << endl;
        }

    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -2;
    }



    return 0;
}
