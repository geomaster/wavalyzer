#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "wav.hpp"
#include "fft.hpp"
#include "window.hpp"
#include "gui.hpp"
#include "histogram.hpp"

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
                    cout << "M";
                } else if (samples[2*j] >= lower - 0.01f && samples[2*j] <= upper + 0.01f){
                    cout << "X";
                } else if (samples[2*j] >= lower - 0.02f && samples[2*j] <= upper + 0.02f) {
                    cout << "x";
                } else if (samples[2*j] >= lower - 0.04f && samples[2*j] <= upper + 0.04f) {
                    cout << ":";
                } else if (samples[2*j] >= lower - 0.08f && samples[2*j] <= upper + 0.08f) {
                    cout << ".";
                } else {
                    cout << " ";
                }
            }

            cout << endl;
        }

        vector<wavalyzer::fft_result_t> ffts;
        while (w.get_total_samples() - w.get_samples_read() > 1024) {
            samples.clear();
            w.read_samples(samples, 1024);
            wavalyzer::apply_hann_window(samples);
            ffts.push_back(wavalyzer::fft_from_samples(samples, w.get_sample_rate(), 50, 200, 2400, 1.0f / wavalyzer::get_hann_window_gain()));
        }

        for (int i = 0; i < (2400 - 200) / 50; i++) {
            cout << setw(4) << (200 + 50*i) << "Hz ";
            for (int j = 0; j < min(ffts.size(), (size_t)200); j++) {
                float bin_val = ffts[j][i];
                if (bin_val >= 1.0f) {
                    cout << "!";
                } else if (bin_val >= 0.8f) {
                    cout << "M";
                } else if (bin_val >= 0.64f) {
                    cout << "X";
                } else if (bin_val >= 0.48f) {
                    cout << "x";
                } else if (bin_val >= 0.32f) {
                    cout << ":";
                } else if (bin_val >= 0.16f) {
                    cout << ".";
                }  else {
                    cout << " ";
                }

            }

            cout << endl;
        }


        {
            wavalyzer::gui::histogram hist(ffts[ffts.size() / 4], 200, 2400, 50, 15);
            wavalyzer::gui::diagram_window window(&hist);
            window.start();
        }
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -2;
    }



    return 0;
}
