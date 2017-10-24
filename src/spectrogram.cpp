#include "spectrogram.hpp"
#include "common.hpp"
#include <iostream>
#include <cstring>

using namespace std;
using namespace wavalyzer::gui;
using namespace wavalyzer;

const int MIN_MS_IN_VIEW = 5;
const int Y_LABEL_COUNT = 15;
const int X_LABEL_COUNT = 10;

const int COLOR_STOP_COUNT = 6;

const int COLOR_STOPS[6][3] = { {  34,  34,  34 },
                                {   0,   0, 255 },
                                { 128,   0, 128 },
                                { 255,   0,   0 },
                                { 255, 255,   0 },
                                { 255, 255, 255 } };

const float DBFS_STOPS[] = { -80.0f, -64.0f, -48.0f, -32.0f, -16.0f, 0.0f };
const float GAIN_DBFS = 15.0f;

spectrogram::spectrogram(const std::vector<fft_result_t>& _fft_results,
                         int _step_ms,
                         float _min_hertz,
                         float _max_hertz,
                         float _step_hertz) :

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

    // Transpose the FFT matrix to imrpove cache locality when
    // rendering, and also convert the sample levels to dBFS to
    // avoid expensive log() calls inside hot loops
    size_t fft_h = _fft_results.size(),
           fft_w = _fft_results[0].size();

    fft_matrix_w = fft_h;

    fft_dbfs = new float[fft_w * fft_h];
    for (size_t i = 0; i < fft_h; i++) {
        for (size_t j = 0; j < fft_w; j++) {
            fft_dbfs[j * fft_matrix_w + i] = sample_level_to_dbfs(_fft_results[i][j]);
        }
    }
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

float spectrogram::get_x_granularity(float min_drag_step)
{
    if (get_drag_step_normalized() < min_drag_step) {
        return max(static_cast<float>(step_ms), min_drag_step * (right_ms - left_ms));
    } else {
        return step_ms;
    }
}

float spectrogram::get_zoom_granularity()
{
    return max(static_cast<float>(step_ms), static_cast<float>(right_ms - left_ms) * 0.1f);
}

float spectrogram::get_drag_step_normalized()
{
    return static_cast<float>(step_ms) / (right_ms - left_ms);
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
    cached_texture_dirty = true;
}

void spectrogram::render_spectrogram_texture(pair<int, int> size)
{
    if (cached_texture_size != size) {
        if (!cached_texture.create(size.first, size.second)) {
            throw gui_exception("Could not create spectrogram texture");
        }

        cached_texture_size = size;
        pixels.resize(size.first * size.second * 4);
    }

    float hertz_px_step = static_cast<float>(max_hertz - min_hertz) / ((size.second - 1) * step_hertz),
          ms_px_step = static_cast<float>(right_ms - left_ms) / ((size.first - 1) * step_ms),
          left_ms_offset = static_cast<float>(left_ms) / step_ms;
    int fft_w = fft_matrix_w;

    int last_bucket = -1;
    for (int y = 0; y < size.second; y++) {
        int bucket = floor((size.second - 1 - y) * hertz_px_step);

        // Fast path - same bucket, just copy the row above
        if (bucket == last_bucket) {
            int prev_row_start = (y - 1) * size.first * 4,
                this_row_start = y * size.first * 4;

            memcpy(&pixels[this_row_start], &pixels[prev_row_start], size.first * 4);
            continue;
        }

        for (int x = 0; x < size.first; x++) {
            float ms_frac = left_ms_offset + x * ms_px_step;

            int nn_left = static_cast<int>(floor(ms_frac)),
                nn_right = static_cast<int>(ceil(ms_frac));

            if (nn_right >= max_ms) {
                nn_right = max_ms - 1;
            }

            if (nn_left > nn_right) {
                nn_left = nn_right;
            }

            float dbfs_left = fft_dbfs[bucket * fft_w + nn_left],
                  dbfs_right = fft_dbfs[bucket * fft_w + nn_right];

            float nn_value = (ms_frac - nn_left) > 0.5f ? dbfs_right : dbfs_left;

            sf::Color nn_color = color_from_dbfs(nn_value);
            pixels[y * size.first * 4 + x * 4 + 0] = nn_color.r;
            pixels[y * size.first * 4 + x * 4 + 1] = nn_color.g;
            pixels[y * size.first * 4 + x * 4 + 2] = nn_color.b;
            pixels[y * size.first * 4 + x * 4 + 3] = nn_color.a;
        }

        last_bucket = bucket;
    }

    cached_texture.update(&pixels[0]);
}

sf::Color spectrogram::color_from_dbfs(float dbfs)
{
    dbfs += GAIN_DBFS;

    if (dbfs < DBFS_STOPS[0]) {
        return sf::Color(COLOR_STOPS[0][0], COLOR_STOPS[0][1], COLOR_STOPS[0][2], 255);
    }

    int last = COLOR_STOP_COUNT - 1;
    if (dbfs >= DBFS_STOPS[last]) {
        return sf::Color(COLOR_STOPS[last][0], COLOR_STOPS[last][1], COLOR_STOPS[last][2], 255);
    }

    int stop;
    for (stop = 1; stop < COLOR_STOP_COUNT; stop++) {
        if (dbfs < DBFS_STOPS[stop]) {
            break;
        }
    }

    float alpha = 1.0f - (dbfs - DBFS_STOPS[stop - 1]) / (DBFS_STOPS[stop] - DBFS_STOPS[stop - 1]);
    return sf::Color(lerp(COLOR_STOPS[stop - 1][0], COLOR_STOPS[stop][0], alpha),
                     lerp(COLOR_STOPS[stop - 1][1], COLOR_STOPS[stop][1], alpha),
                     lerp(COLOR_STOPS[stop - 1][2], COLOR_STOPS[stop][2], alpha),
                     255);
}

void spectrogram::draw(sf::RenderTarget* target, pair<int, int> bottom_left, pair<int, int> size)
{
    if (cached_texture_size != size || cached_texture_dirty) {
        render_spectrogram_texture(size);
        cached_texture_dirty = false;
    }

    sf::Sprite new_sprite(cached_texture);
    new_sprite.setPosition(sf::Vector2f(bottom_left.first, bottom_left.second - size.second));
    target->draw(new_sprite);
}

spectrogram::~spectrogram()
{
    delete[] fft_dbfs;
}
