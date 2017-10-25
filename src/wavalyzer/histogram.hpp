#pragma once
#include "gui.hpp"
#include <vector>
#include <string>

namespace wavalyzer::gui {
    class histogram : public diagram {
    private:
        std::vector<float> values;
        float min_hertz, max_hertz, step_hertz;
        size_t buckets;

        float left_hertz, right_hertz;
        std::vector<sf::RectangleShape> bars;
        std::vector<float> bar_values;

        void update_bars();
        int hertz_to_index(float hertz);

        float get_bucket_width();
        float get_bucket_width_normalized();
        float get_bucket_start_alpha(int bucket);
        float get_bucket_start_frequency(int bucket);

    public:
        histogram(const std::vector<float>& _values,
                  float _min_hertz,
                  float _max_hertz,
                  float _step_hertz,
                  size_t _buckets);

        std::string get_title();
        std::string get_message();

        float get_drag_step_normalized();

        std::map<float, std::string> get_y_labels();
        std::pair<float, float> get_full_x_range();
        float get_min_x_width();
        float get_x_granularity(float);
        std::map<float, std::string> get_x_labels();
        void set_x_range(std::pair<float, float> new_range);
        void draw(sf::RenderTarget* target, std::pair<int, int> bottom_left, std::pair<int, int> size);

        virtual ~histogram() {}
    };
}
