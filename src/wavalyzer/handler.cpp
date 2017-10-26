#include "handler.hpp"
#include "sfml_pdf.hpp"
#include <cmath>
#include <iostream>

using namespace std;
using namespace wavalyzer;
using namespace wavalyzer::gui;

main_diagram_event_handler::main_diagram_event_handler(const vector<fft_result_t>& _ffts,
                                                       int _min_freq,
                                                       int _max_freq,
                                                       int _step_freq,
                                                       int _step_ms,
                                                       int _histogram_buckets) :

                                                       diagram_event_handler(),
                                                       ffts(_ffts),
                                                       min_freq(_min_freq),
                                                       max_freq(_max_freq),
                                                       step_freq(_step_freq),
                                                       step_ms(_step_ms),
                                                       histogram_buckets(_histogram_buckets),
                                                       hist(nullptr),
                                                       spect(nullptr),
                                                       save_counter(0)
{
    spect = new spectrogram(ffts, step_ms, min_freq, max_freq, step_freq);
}

void main_diagram_event_handler::on_click_mark(float x)
{
    if (hist != nullptr) {
        return;
    }

    int ms_nearest = round(x / step_ms);
    if (hist != nullptr) {
        delete hist;
        hist = nullptr;
    }

    hist = new histogram(ffts[ms_nearest], min_freq, max_freq, step_freq, histogram_buckets);
    parent->set_diagram(hist);
}

void main_diagram_event_handler::set_parent(diagram_window* new_parent)
{
    parent = new_parent;
    parent->set_diagram(spect);
}

void main_diagram_event_handler::on_key_press(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::BackSpace && hist != nullptr) {
        delete hist;
        hist = nullptr;
        parent->set_diagram(spect);
    } else if (key == sf::Keyboard::P) {
        sfml_pdf pdf;
        cout << "Please wait, rendering the diagram..." << endl;

        if (hist != nullptr) {
            pdf.draw_diagram(hist, false);
        } else {
            pdf.draw_diagram(spect, true);
        }

        string filename = "diagram_" + to_string(++save_counter) + ".pdf";
        try {
            pdf.save_to_file(filename);
            cerr << "Saved to file`" << filename << "`" << endl;
        } catch (std::exception &e) {
            cerr << "Failed saving file `" << filename << "`: " << e.what() << endl;
        }
    }
}

main_diagram_event_handler::~main_diagram_event_handler()
{
    if (hist != nullptr) {
        delete hist;
    }

    if (spect != nullptr) {
        delete spect;
    }
}
