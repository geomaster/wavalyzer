#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "wav.hpp"
#include "fft.hpp"
#include "window.hpp"
#include "gui.hpp"
#include "histogram.hpp"
#include "spectrogram.hpp"

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
        w.read_samples(samples, w.get_total_samples());
        float ms_samples = w.get_sample_rate() / 1000.0f;
        int total_ms = w.get_total_samples() / ms_samples;

        vector<wavalyzer::fft_result_t> ffts;
        int window_size = 2048;
        int min_freq = 100;
        int max_freq = 2000;
        int freq_step = 1;
        int report_ms_interval = total_ms / 20;
        float ms_per_window = window_size / ms_samples;

        cout << "Analyzing. This may take a while.\n";

        vector<float> window_samples(window_size);
        for (int i = ms_per_window / 2; i < total_ms - ms_per_window / 2; i++) {
            int left_sample = (i - ms_per_window / 2) * ms_samples;
            int right_sample = (i + ms_per_window / 2) * ms_samples;

            for (int j = left_sample, sample = 0; j < right_sample; j++, sample++) {
                if (sample < 0 || sample >= window_size) {
                    break;
                }

                window_samples[sample] = samples[j];
            }

            wavalyzer::apply_hann_window(window_samples);
            ffts.push_back(wavalyzer::fft_from_samples(window_samples,
                                                       w.get_sample_rate(),
                                                       freq_step,
                                                       min_freq,
                                                       max_freq,
                                                       1.0f / wavalyzer::get_hann_window_gain()));

            if (i % report_ms_interval == 0) {
                cout << fixed << "Analyzed " <<
                    i << "ms of " << total_ms - ms_per_window << "ms ("
                    << setprecision(2) << static_cast<float>(100 * i) / (total_ms - ms_per_window) << " %)" << endl;
            }
        }

        {
            wavalyzer::gui::histogram hist(ffts[ffts.size() / 4], min_freq, max_freq, freq_step, 15);
            wavalyzer::gui::spectrogram spect(ffts, 1, min_freq, max_freq, freq_step);

            wavalyzer::gui::diagram_window window(&spect);
            window.start();
        }
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -2;
    }

    return 0;
}
