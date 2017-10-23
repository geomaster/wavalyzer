#include "spectrogram.hpp"
#include "common.hpp"

using namespace std;
using namespace wavalyzer::gui;
using namespace wavalyzer;

const int MIN_MS_IN_VIEW = 5;
const int Y_LABEL_COUNT = 15;
const int X_LABEL_COUNT = 10;

spectrogram::spectrogram(const std::vector<fft_result_t>& _fft_results,
                         int _step_ms,
                         float _min_hertz,
                         float _max_hertz,
                         float _step_hertz) :

                         fft(_fft_results),
                         step_ms(_step_ms),
                         min_hertz(_min_hertz),
                         max_hertz(_max_hertz),
                         step_hertz(_step_hertz),
                         left_ms(0),
                         max_ms((_fft_results.size() - 1) * _step_ms),
                         cached_texture(),
                         cached_texture_size(0, 0),
                         cached_texture_dirty(true)
{
    right_ms = max_ms - 1;
    sprite.setTexture(cached_texture);
}

map<float, string> spectrogram::get_y_labels()
{
    map<float, string> labels;
    for (int i = 0; i < Y_LABEL_COUNT; i++) {
        float alpha = static_cast<float>(i) / (Y_LABEL_COUNT - 1);
        int hertz = min_hertz + alpha * (max_hertz - min_hertz);
        labels.insert(make_pair(alpha, hertz_to_string(hertz)));
    }

    return labels;
}

string spectrogram::get_title()
{
    return "Spectrogram";
}

string spectrogram::get_message()
{
    return "Click on a slice to zoom in";
}

pair<float, float> spectrogram::get_full_x_range()
{
    return make_pair(0.0f, max_ms - 1.0f);
}

float spectrogram::get_min_x_width()
{
    return MIN_MS_IN_VIEW;
}

float spectrogram::get_x_granularity()
{
    return step_ms * 10;
}

map<float, string> spectrogram::get_x_labels()
{
    map<float, string> labels;
    for (int i = 0; i < X_LABEL_COUNT; i++) {
        float alpha = static_cast<float>(i) / (X_LABEL_COUNT - 1);
        int ms = left_ms + alpha * (right_ms - left_ms);
        labels.insert(make_pair(alpha, to_string(ms) + "ms"));
    }

    return labels;
}

void spectrogram::set_x_range(pair<float, float> new_range)
{
    left_ms = new_range.first;
    right_ms = new_range.second;
}

void spectrogram::render_spectrogram_texture(pair<int, int> size)
{
    if (cached_texture_size != size) {
        if (!cached_texture.create(size.first, size.second)) {
            throw gui_exception("Could not create spectrogram texture");
        }

        cached_texture_size = size;
    }

    vector<vector<sf::Color>> columns;
    for (int ms = left_ms - step_ms, i = 0; ms <= right_ms + step_ms && ms < max_ms; ms += step_ms, i++) {
        if (ms < 0) {
            ms = 0;
        }

        vector<sf::Color> column;
        column.reserve(size.second);
        const fft_result_t& fft_result = fft[ms / step_ms];

        for (int y = 0; y < size.second; y++) {
            float alpha = 1.0f - (static_cast<float>(y) / (size.second - 1)),
                  hertz = min_hertz + alpha * (max_hertz - min_hertz);

            int bucket = floor((hertz - min_hertz) / step_hertz);
            float dbfs = sample_level_to_dbfs(fft_result[bucket]);

            column.push_back(color_from_dbfs(dbfs));
        }

        columns.push_back(move(column));
    }

    vector<sf::Uint8> pixels(size.first * size.second * 4);
    for (int y = 0; y < size.second; y++) {
        for (int x = 0; x < size.first; x++) {
            float alpha = static_cast<float>(x) / (size.first - 1),
                  ms_frac = alpha * (right_ms - left_ms) / step_ms;

            int lerp_left = static_cast<int>(floor(ms_frac)),
                lerp_right = static_cast<int>(ceil(ms_frac));

            if (lerp_right >= max_ms) {
                lerp_right = max_ms - 1;
            }

            if (lerp_left > lerp_right) {
                // TODO: Don't ignore undersampling?
                lerp_left = lerp_right;
            }

            float lerp_alpha = 1.0f - (ms_frac - lerp_left);

            sf::Color color_left = columns[lerp_left][y],
                      color_right = columns[lerp_right][y];

            sf::Color lerp_color = sf::Color(lerp(color_left.r, color_right.r, lerp_alpha),
                                             lerp(color_left.g, color_right.g, lerp_alpha),
                                             lerp(color_left.b, color_right.b, lerp_alpha),
                                             255);

            //pixels[y * size.first + x] = lerp_color.toInteger() | 0x000000ff;
            pixels[y * size.first * 4 + x * 4 + 0] = lerp_color.r;
            pixels[y * size.first * 4 + x * 4 + 1] = lerp_color.g;
            pixels[y * size.first * 4 + x * 4 + 2] = lerp_color.b;
            pixels[y * size.first * 4 + x * 4 + 3] = lerp_color.a;

        }
    }

    cached_texture.update(&pixels[0]);
}

sf::Color spectrogram::color_from_dbfs(float dbfs)
{
    // TODO: Non-greyscale color coding
    int grey_level = 255.0f * (1.0f - (dbfs / -96.0f));

    return sf::Color(grey_level, grey_level, grey_level, 255);
}

void spectrogram::draw(sf::RenderTarget* target, pair<int, int> bottom_left, pair<int, int> size)
{
    if (cached_texture_size != size || cached_texture_dirty) {
        render_spectrogram_texture(size);
    }

    sf::Sprite new_sprite(cached_texture);
    new_sprite.setPosition(sf::Vector2f(bottom_left.first, bottom_left.second - size.second));
    target->draw(new_sprite);
}
