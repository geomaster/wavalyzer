#pragma once
#include <utility>
#include <memory>
#include <string>
#include <map>
#include <exception>
#include <SFML/Graphics.hpp>

namespace wavalyzer::gui {
    class gui_exception : public std::exception {
    private:
        std::string message;

    public:
        gui_exception(const std::string& _message)
            : message(_message) {}

        gui_exception(const std::string&& _message)
            : message(std::move(_message)) {}

        virtual const char* what() const throw() {
            return message.c_str();
        }
    };

    class diagram {
    public:
        virtual std::map<float, std::string> get_y_labels() = 0;
        virtual std::string get_title() { return "Diagram"; }
        virtual std::string get_message() { return ""; }

        virtual std::pair<float, float> get_full_x_range() = 0;
        virtual float get_min_x_width() = 0;
        virtual float get_x_granularity() = 0;
        virtual std::map<float, std::string> get_x_labels() = 0;

        virtual void set_x_range(std::pair<float, float> new_range) = 0;

        virtual void draw(sf::RenderTarget* target, std::pair<int, int> bottom_left, std::pair<int, int> size) = 0;
    };

    class diagram_window {
    private:
        diagram* diag;
        std::unique_ptr<sf::RenderWindow> window;
        sf::Font font;
        std::vector<sf::Text> x_labels, y_labels;
        std::vector<sf::RectangleShape> x_etches, y_guides;
        std::pair<float, float> x_range, drag_start_x_range;
        sf::RectangleShape x_axis, y_axis, horizontal_rule;
        sf::Text title, message;
        bool dirty, dragging;
        int drag_start_x;

        std::pair<float, float> check_range(std::pair<float, float> range);

        void create_x_labels();
        void create_y_labels();
        void create_axes();
        void create_texts();

        void draw_axes();
        void draw_labels();
        void draw_texts();

        void handle_wheel(sf::Event::MouseWheelScrollEvent& event);
        void handle_mouse_down(sf::Event::MouseButtonEvent& event);
        void handle_mouse_up(sf::Event::MouseButtonEvent& event);
        void handle_mouse_move(sf::Event::MouseMoveEvent& event);

    public:
        diagram_window(diagram* _diagram);

        void start();
    };
}
