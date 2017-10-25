#pragma once
#include "gui.hpp"
#include "fft.hpp"
#include "histogram.hpp"
#include "spectrogram.hpp"
#include <vector>

namespace wavalyzer::gui {
    class main_diagram_event_handler : public diagram_event_handler {
    private:
        const std::vector<fft_result_t>& ffts;
        int min_freq, max_freq, step_freq, step_ms, histogram_buckets;
        histogram *hist;
        spectrogram *spect;

    public:
        main_diagram_event_handler(const std::vector<fft_result_t>& _ffts,
                                   int _min_freq,
                                   int _max_freq,
                                   int _step_freq,
                                   int _step_ms,
                                   int _histogram_buckets);

        virtual void set_parent(diagram_window* new_parent);
        virtual void on_click_mark(float x);
        virtual void on_key_press(sf::Keyboard::Key key);

        virtual ~main_diagram_event_handler();
    };
}
