#pragma once
#include "gui.hpp"
#include "fft.hpp"

namespace wavalyzer::gui {
    class spectrogram : public diagram {
    private:
        std::vector<fft_result_t> fft;
        int step_ms;
        float min_hertz, max_hertz, step_hertz;
        int left_ms, right_ms, max_ms;
        sf::Texture cached_texture;
        sf::Sprite sprite;
        std::pair<int, int> cached_texture_size;
        bool cached_texture_dirty;

        void render_spectrogram_texture(std::pair<int, int> size);
        sf::Color color_from_dbfs(float dbfs);

    public:
        spectrogram(const std::vector<fft_result_t>& _fft_results,
                    int _step_ms,
                    float _min_hertz,
                    float _max_hertz,
                    float _step_hertz);

        std::map<float, std::string> get_y_labels();
        std::string get_title();
        std::string get_message();

        std::pair<float, float> get_full_x_range();
        float get_min_x_width();
        float get_x_granularity();
        std::map<float, std::string> get_x_labels();

        void set_x_range(std::pair<float, float> new_range);

        void draw(sf::RenderTarget* target, std::pair<int, int> bottom_left, std::pair<int, int> size);
    };
}
