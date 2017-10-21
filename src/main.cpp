#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "wav.hpp"
#include "fft.hpp"
#include "window.hpp"

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
        w.read_samples(samples, 4096);

        for (int i = -15; i <= 15; i++) {
            float lower = (float)i / 15.0f - 0.5f / 15.0f, upper = (float)i / 15.0f + 0.5f / 15.0f;

            for (int j = 0; j < 400 / 2; j++) {
                if (samples[2*j] >= lower && samples[2*j] <= upper) {
                    cout << ":";
                } else {
                    cout << " ";
                }
            }

            cout << endl;
        }

        vector<wavalyzer::fft_result_t> ffts;
        while (w.get_total_samples() - w.get_samples_read() > 256) {
            samples.clear();
            w.read_samples(samples, 256);
            wavalyzer::apply_hamming_window(samples);
            ffts.push_back(wavalyzer::fft_from_samples(samples, w.get_sample_rate(), 50, 400, 2400));
        }

        for (int i = 0; i < (2400 - 400) / 50; i++) {
            cout << setw(4) << (400 + 50*i) << "Hz ";
            for (int j = 0; j < ffts.size(); j++) {
                float bin_val = ffts[j][i];
                if (bin_val >= 4.0f) {
                    cout << "M";
                } else if (bin_val >= 2.0f) {
                    cout << "X";
                } else if (bin_val >= 1.0f) {
                    cout << "x";
                } else if (bin_val >= 0.5f) {
                    cout << ":";
                } else if (bin_val >= 0.25f) {
                    cout << ".";
                }  else {
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
