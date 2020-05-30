#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "wav.hpp"
#include "fft.hpp"
#include "window.hpp"
#include "gui.hpp"
#include "handler.hpp"

using namespace std;

struct config_t {
    config_t() : window_size(1024),
                 hamming(false),
                 min_freq(100),
                 max_freq(2000),
                 freq_step(10),
                 ms_step(1),
                 buckets(15),
                 filename("")

    {
    }

    size_t window_size;
    bool hamming;
    size_t min_freq;
    size_t max_freq;
    size_t freq_step;
    size_t ms_step;
    size_t buckets;
    string filename;
};

size_t as_number(const string& s)
{
    size_t n = 0;
    if (s.empty()) {
        return -1;
    }

    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] < '0' || s[i] > '9') {
            return -1;
        }

        n = n * 10 + (s[i] - '0');
    }

    return n;
}

bool config_validate(const config_t& c)
{
    switch (c.window_size) {
    case (1 << 7):
    case (1 << 8):
    case (1 << 9):
    case (1 << 10):
    case (1 << 11):
    case (1 << 12):
    case (1 << 13):
    case (1 << 14):
        break;

    default:
        cerr << "Window size must be a power of two between 128 and 16384." << endl;
        return false;
    }

    if (c.ms_step <= 0 || c.ms_step > 1000) {
        cerr << "Time step must be between 1ms and 1000ms." << endl;
        return false;
    }

    if (c.freq_step <= 0 || c.freq_step > 1000) {
        cerr << "Frequency step must be between 1Hz and 1000Hz." << endl;
        return false;
    }

    if (c.min_freq > 20000) {
        cerr << "Minimum frequency must be below 20kHz." << endl;
        return false;
    }

    if (c.max_freq > 20000) {
        cerr << "Maximum frequency must be below 20kHz." << endl;
        return false;
    }

    if (c.buckets <= 0 || c.buckets > 20) {
        cerr << "Bucket count must be between 1 and 20." << endl;
        return false;
    }

    return true;
}

bool arg_parse(int argc, char* argv[], config_t& res)
{
    int i;
    bool got_filename = false;
    for (i = 1; i < argc; i++) {
        string option = argv[i];
        if (option.empty()) {
            continue;
        }

        if (option[0] == '-') {
            if (option.size() != 2 || i >= argc - 2) {
                cout << "Option syntax error on argument " << i << endl;
                return false;
            }

            string next = argv[i + 1], window_type;
            size_t colon_index, hyphen_index;
            switch (option[1]) {
            case 'w':
                colon_index = next.find(':');
                if (colon_index != string::npos) {
                    res.window_size = as_number(next.substr(0, colon_index));
                    window_type = next.substr(colon_index + 1);
                    if (window_type == "hamming") {
                        res.hamming = true;
                    } else if (window_type == "hann") {
                        res.hamming = false;
                    } else {
                        cerr << "Unknown window type." << endl;
                        return false;
                    }
                } else {
                    res.window_size = as_number(next);
                }

                break;

            case 'f':
                hyphen_index = next.find('-');
                if (hyphen_index == string::npos) {
                    cerr << "Invalid format for frequency range." << endl;
                    return false;
                }

                res.min_freq = as_number(next.substr(0, hyphen_index));
                res.max_freq = as_number(next.substr(hyphen_index + 1));

                break;

            case 'r': res.freq_step = as_number(next); break;
            case 't': res.ms_step = as_number(next); break;
            case 'b': res.buckets = as_number(next); break;
            default: cerr << "Invalid option " << option << endl; return false;
            }

            i++;
        } else {
            if (i != argc - 1) {
                cerr << "Extra parameters after filename." << endl;
                return false;
            }

            res.filename = option;
            got_filename = true;
        }
    }

    return got_filename;
}

int main(int argc, char* argv[])
{
    config_t conf;
    bool bad_command_line = argc < 2;
    if (!bad_command_line) {
        if (!arg_parse(argc, argv, conf) || !config_validate(conf)) {
            cerr << "Invalid command line." << endl << endl;
            bad_command_line = true;
        }
    } else {
        cerr << "Too few arguments." << endl << endl;
    }

    if (bad_command_line) {
        cerr << "Usage: " << argv[0] << " [options] <wavfile>" << endl <<
                endl <<
                "Valid options are:" << endl <<
                "    -w size[:hamming|hann]   Window size and type." << endl <<
                "    -f min-max               Frequency range (both in Hz)." << endl <<
                "    -r resolution            Frequency resolution (in Hz)." << endl <<
                "    -t resolution            Time resolution (in ms)." << endl <<
                "    -b buckets               Number of histogram buckets." << endl;

        return -1;
    }

    try {
        ifstream f(conf.filename);
        wavalyzer::wav_file w(f);

        cout << "[+] File `" << conf.filename << "` loaded!" << endl <<
                "[|] Channels: " << w.get_channels() << endl <<
                "[|] Total samples: " << w.get_total_samples() << endl <<
                "[|] Sample rate: " << w.get_sample_rate() << endl;

        vector<float> samples;
        w.read_samples(samples, w.get_total_samples());
        float ms_samples = w.get_sample_rate() / 1000.0f;
        int total_ms = w.get_total_samples() / ms_samples;

        vector<wavalyzer::fft_result_t> ffts;
        int window_size = static_cast<int>(conf.window_size);
        int min_freq = static_cast<int>(conf.min_freq);
        int max_freq = static_cast<int>(conf.max_freq);
        int freq_step = static_cast<int>(conf.freq_step);
        int buckets = static_cast<int>(conf.buckets);
        int ms_step = static_cast<int>(conf.ms_step);

        int report_ms_interval = total_ms / (20 * ms_step);
        // Prevent a SIGFPE if the input file is short enough to make this 0
        if (report_ms_interval == 0)
            report_ms_interval = 1;
        float ms_per_window = window_size / ms_samples;

        cout << "[+] Analyzing. This may take a while.\n";

        float gain_compensation = conf.hamming ? 1.0f / wavalyzer::get_hamming_window_gain() :
                                                   1.0f / wavalyzer::get_hann_window_gain();

        vector<float> window_samples(window_size);

        for (int i = ceil(ms_per_window / 2), counter = 0;
             i < floor(total_ms - ms_per_window / 2);
             i += ms_step, counter++) {

            int left_sample = (i - ms_per_window / 2) * ms_samples;
            int right_sample = (i + ms_per_window / 2) * ms_samples;

            for (int j = left_sample, sample = 0; j < right_sample; j++, sample++) {
                if (sample < 0 || sample >= window_size) {
                    break;
                }

                window_samples[sample] = samples[j];
            }

            if (conf.hamming) {
                wavalyzer::apply_hamming_window(window_samples);
            } else {
                wavalyzer::apply_hann_window(window_samples);
            }

            ffts.push_back(wavalyzer::fft_from_samples(window_samples,
                                                       w.get_sample_rate(),
                                                       freq_step,
                                                       min_freq,
                                                       max_freq,
                                                       gain_compensation));

            if (counter % report_ms_interval == 0) {
                cout << fixed << "[|] Analyzed " <<
                    i << "ms of " << total_ms - ms_per_window << "ms ("
                    << setprecision(2) << static_cast<float>(100 * i) / (total_ms - ms_per_window) << " %)" << endl;
            }
        }

        wavalyzer::gui::diagram_window window(nullptr);
        wavalyzer::gui::main_diagram_event_handler handler(ffts, min_freq, max_freq, freq_step, ms_step, buckets);

        cout << endl <<
                "[+] GUI running!" << endl <<
                "[|] Use the mouse wheel to zoom into areas of the plot." << endl <<
                "[|] Click and drag to pan around the plot." << endl <<
                "[|] In the spectrogram view, click somewhere on the plot to view the " << endl <<
                "    histogram of that instant." << endl <<
                "[|] When in the histogram view, press Backspace to go back to viewing" << endl <<
                "    the spectrogram." << endl <<
                "[|] Press the P key at any time to save a PDF of the current contents" << endl <<
                "    of the screen. The PDFs will be saved as diagram_1.pdf," << endl <<
                "    diagram_2.pdf, etc. in your current working directory." << endl << endl;

        window.set_event_handler(&handler);
        window.start();
    } catch (exception& e) {
        cerr << "[-] An error has occurred: " << e.what() << endl;
        return -1;
    }

    return 0;
}
