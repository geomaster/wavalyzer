#include "histogram.hpp"
#include <cmath>
#include <iostream>
#include "common.hpp"

using namespace wavalyzer;
using namespace wavalyzer::gui;
using namespace std;

const int Y_LABEL_STEP = 12;
const int MAX_Y = 100;
const uint32_t BAR_COLOR = 0xf77a1bff;

histogram::histogram(const std::vector<float>& _values,
                     float _min_hertz,
                     float _max_hertz,
                     float _step_hertz,
                     size_t _buckets) :

                     values(_values),
                     min_hertz(_min_hertz),
                     max_hertz(_max_hertz),
                     step_hertz(_step_hertz),
                     buckets(_buckets),
                     left_hertz(_min_hertz),
                     right_hertz(_max_hertz),
                     bars(_buckets),
                     bar_values(_buckets)
{
    for (sf::RectangleShape& bar : bars) {
        bar.setFillColor(sf::Color(BAR_COLOR));
    }
}

map<float, string> histogram::get_y_labels()
{
    map<float, string> labels;
    for (int i = -96; i <= 0; i += Y_LABEL_STEP) {
        labels.insert(make_pair(static_cast<float>(i) / 96.0f + 1.0f, to_string(i) + "dBFS"));
    }

    return labels;
}

float histogram::get_drag_step_normalized()
{
    return get_bucket_width_normalized();
}

float histogram::get_min_x_width()
{
    return buckets * step_hertz;
}

string histogram::get_title()
{
    return "Histogram";
}

string histogram::get_message()
{
    return "Press Backspace to go back";
}

pair<float, float> histogram::get_full_x_range()
{
    return make_pair(min_hertz, max_hertz);
}

float histogram::get_x_granularity(float)
{
    return get_bucket_width();
}

float histogram::get_bucket_width()
{
    return (right_hertz - left_hertz) / buckets;
}

float histogram::get_bucket_width_normalized()
{
    return 1.0f / buckets;
}

float histogram::get_bucket_start_alpha(int bucket)
{
    float w = get_bucket_width_normalized();
    return bucket * w + w / 2.0f;
}

float histogram::get_bucket_start_frequency(int bucket)
{
    float w = get_bucket_width();
    return left_hertz + bucket * w + w / 2.0f;
}

map<float, string> histogram::get_x_labels()
{
    map<float, string> labels;
    float bucket_width = get_bucket_width();

    for (size_t i = 0; i < buckets; i++) {
        float label_x = get_bucket_start_alpha(i);
        float bucket_middle_freq = get_bucket_start_frequency(i) + bucket_width / 2.0f;

        labels.insert(make_pair(label_x, hertz_to_string(bucket_middle_freq)));
    }

    return labels;
}

void histogram::set_x_range(pair<float, float> new_range)
{
    left_hertz = new_range.first;
    right_hertz = new_range.second;

    update_bars();
}

int histogram::hertz_to_index(float hertz)
{
    if (hertz < min_hertz || hertz > max_hertz) {
        return -1;
    }

    hertz -= min_hertz;
    hertz /= step_hertz;

    return static_cast<int>(hertz);
}

void histogram::update_bars()
{
    float bucket_width = get_bucket_width();
    for (size_t i = 0; i < buckets; i++) {
        float bucket_left_freq = get_bucket_start_frequency(i),
              bucket_right_freq = bucket_left_freq + bucket_width;

        int bucket_left_index = hertz_to_index(bucket_left_freq),
            bucket_right_index = hertz_to_index(bucket_right_freq);

        if (bucket_left_index < 0) {
            bucket_left_index = 0;
        }

        if (bucket_left_index >= values.size()) {
            bucket_left_index = values.size() - 1;
        }

        if (bucket_right_index < 0) {
            bucket_right_index = 0;
        }

        if (bucket_right_index >= values.size()) {
            bucket_right_index = values.size() - 1;
        }

        float sum = 0.0f;
        for (int j = bucket_left_index; j <= bucket_right_index; j++) {
            sum += values[j];
        }

        float value = sum / (bucket_right_index - bucket_left_index + 1);
        bar_values[i] = sample_level_to_dbfs(value);
    }
}

void histogram::update_bar_shapes(pair<int, int> bottom_left, pair<int, int> size)
{
    float bucket_width_n = get_bucket_width_normalized();
    float bucket_width_px = bucket_width_n * size.first;
    float bar_width = bucket_width_px / 2.0f;

    for (size_t i = 0; i < buckets; i++) {
        float value_scaled = (bar_values[i] + 96.0f) / 96.0f;
        float x = bottom_left.first + (i * bucket_width_px + bucket_width_px / 2.0f);
        float y = bottom_left.second - value_scaled * size.second;

        bars[i].setSize(sf::Vector2f(bar_width, value_scaled * size.second));
        bars[i].setPosition(sf::Vector2f(x - bar_width / 2.0f, y));
    }
}

void histogram::draw(sf::RenderTarget* target, pair<int, int> bottom_left, pair<int, int> size)
{
    update_bar_shapes(bottom_left, size);
    for (size_t i = 0; i < buckets; i++) {
        target->draw(bars[i]);
    }
}

void histogram::draw_to_pdf(sfml_pdf& pdf, pair<int, int> bottom_left, pair<int, int> size)
{
    update_bar_shapes(bottom_left, size);
    for (size_t i = 0; i < buckets; i++) {
        pdf.draw(bars[i]);
    }
}
