#include "gui.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

using namespace wavalyzer::gui;
using namespace std;
#include <iostream>

const int MAX_CLICK_DISTANCE = 5;

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 600;
const int VRULE_WEIGHT = 2;

const int ZERO_Y = 545;
const int ONE_Y = 80;

const int TITLE_FONT_SIZE = 20;
const int TITLE_Y = 10;
const int TITLE_X = 25;
const int MSG_END_X = 1175;

const int ZERO_X = 100;
const int ONE_X = 1175;

const int HRULE_Y = 50;
const int HRULE_WEIGHT = 1;

const int LABEL_AXIS_GAP = 10;

const float AXIS_WEIGHT = 2.0f;
const float ETCH_WEIGHT = 2.0f;
const float ETCH_HEIGHT = 10.0f;
const float GUIDE_WEIGHT = 1.0f;

const int X_LABEL_FONT_SIZE = 15;
const int Y_LABEL_FONT_SIZE = 15;

const uint32_t BACK_COLOR =  0x222222ff;
const uint32_t AXIS_COLOR =  0xffffffff;
const uint32_t GUIDE_COLOR = 0x666666ff;
const uint32_t VRULE_COLOR = 0xff5000af;

const uint32_t TITLE_COLOR = 0xffffffff;
const uint32_t MSG_COLOR =   0x666666ff;

diagram_window::diagram_window(diagram* _diagram) : handler(nullptr),
                                                    min_drag_step(1.0f / (ONE_X - ZERO_X))

{
    window = make_unique<sf::RenderWindow>(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "wavalyzer gui");
    if (!font.loadFromFile("assets/FiraMono-Regular.otf")) {
        throw gui_exception("Cannot load font");
    }

    if (_diagram != nullptr) {
        set_diagram(_diagram);
    }
}

void diagram_window::set_diagram(diagram* new_diagram)
{
    diag = new_diagram;
    dragging = false;
    drag_start_x = 0;
    dirty = true;
    mouse_x = 0;
    click_mark_x = 0;

    x_range = diag->get_full_x_range();
    diag->set_x_range(x_range);

    create_x_labels();
    create_y_labels();
    create_axes();
    create_texts();

}

void diagram_window::create_y_labels()
{
    map<float, string> labels = diag->get_y_labels();
    y_labels.clear();
    y_guides.clear();

    for (const pair<float, string>& label : labels) {
        int y = static_cast<int>(label.first * (ONE_Y - ZERO_Y) + ZERO_Y);
        sf::RectangleShape guide(sf::Vector2f(ONE_X - ZERO_X, GUIDE_WEIGHT));
        guide.setFillColor(sf::Color(GUIDE_COLOR));
        guide.setPosition(sf::Vector2f(ZERO_X, y));

        sf::Text label_text(label.second, font, Y_LABEL_FONT_SIZE);
        label_text.setFillColor(sf::Color::White);
        label_text.setPosition(sf::Vector2f(ZERO_X - LABEL_AXIS_GAP, y));

        sf::FloatRect label_rect = label_text.getLocalBounds();
        label_text.move(-label_rect.width, -label_rect.height / 2.0f);

        y_labels.push_back(label_text);
        y_guides.push_back(guide);
    }
}

void diagram_window::create_x_labels()
{
    map<float, string> labels = diag->get_x_labels();
    x_labels.clear();
    x_etches.clear();

    for (const pair<float, string>& label : labels) {
        int x = static_cast<int>(label.first * (ONE_X - ZERO_X) + ZERO_X);
        sf::RectangleShape etch(sf::Vector2f(ETCH_WEIGHT, ETCH_HEIGHT));
        etch.setFillColor(sf::Color::White);
        etch.setPosition(sf::Vector2f(x, ZERO_Y - ETCH_HEIGHT / 2));

        sf::Text label_text(label.second, font, X_LABEL_FONT_SIZE);
        label_text.setFillColor(sf::Color::White);
        label_text.setPosition(sf::Vector2f(x, ZERO_Y + LABEL_AXIS_GAP));
        label_text.move(sf::Vector2f(-label_text.getLocalBounds().width / 2.0f, 0.0f));

        x_labels.push_back(label_text);
        x_etches.push_back(etch);
    }
}

void diagram_window::create_texts()
{
    title.setFont(font);
    message.setFont(font);

    title.setCharacterSize(TITLE_FONT_SIZE);
    message.setCharacterSize(TITLE_FONT_SIZE);

    title.setString(diag->get_title());
    message.setString(diag->get_message());

    title.setFillColor(sf::Color(TITLE_COLOR));
    message.setFillColor(sf::Color(MSG_COLOR));

    title.setPosition(sf::Vector2f(TITLE_X, TITLE_Y));
    message.setPosition(sf::Vector2f(MSG_END_X, TITLE_Y));
    message.move(sf::Vector2f(-message.getLocalBounds().width, 0.0f));
}

void diagram_window::draw_texts()
{
    window->draw(title);
    window->draw(message);
}

void diagram_window::create_axes()
{
    vertical_rule.setSize(sf::Vector2f(VRULE_WEIGHT, ZERO_Y - ONE_Y));
    vertical_rule.setFillColor(sf::Color(VRULE_COLOR));
    vertical_rule.setPosition(sf::Vector2f(-WINDOW_WIDTH, -WINDOW_HEIGHT));

    horizontal_rule.setSize(sf::Vector2f(MSG_END_X - TITLE_X, HRULE_WEIGHT));
    horizontal_rule.setFillColor(sf::Color(AXIS_COLOR));
    horizontal_rule.setPosition(sf::Vector2f(TITLE_X, HRULE_Y));

    x_axis.setSize(sf::Vector2f(ONE_X - ZERO_X, AXIS_WEIGHT));
    y_axis.setSize(sf::Vector2f(ZERO_Y - ONE_Y, AXIS_WEIGHT));
    x_axis.setFillColor(sf::Color(AXIS_COLOR));
    y_axis.setFillColor(sf::Color(AXIS_COLOR));

    y_axis.setRotation(90.0f);
    x_axis.setPosition(sf::Vector2f(ZERO_X, ZERO_Y));
    y_axis.setPosition(sf::Vector2f(ZERO_X, ONE_Y));
}

void diagram_window::draw_axes()
{
    window->draw(vertical_rule);
    window->draw(horizontal_rule);
    window->draw(x_axis);
    window->draw(y_axis);
}

void diagram_window::draw_labels()
{
    for (const sf::Text& label : x_labels) {
        window->draw(label);
    }

    for (const sf::RectangleShape& etch : x_etches) {
        window->draw(etch);
    }

    for (const sf::Text& label : y_labels) {
        window->draw(label);
    }

    for (const sf::RectangleShape& guide : y_guides) {
        window->draw(guide);
    }
}

pair<float, float> diagram_window::check_range(pair<float, float> range)
{
    if (range.first < diag->get_full_x_range().first && range.second > diag->get_full_x_range().second) {
        return diag->get_full_x_range();
    }

    if (range.first < diag->get_full_x_range().first) {
        range.first = diag->get_full_x_range().first;
    }

    if (range.second > diag->get_full_x_range().second) {
        range.second = diag->get_full_x_range().second;
    }

    if (range.second - range.first < diag->get_min_x_width()) {
        // Construct a range centered around the current center but with the
        // minimum width
        float center = (range.second + range.first) / 2.0f,
              w2 = diag->get_min_x_width() / 2.0f;

        return make_pair(center - w2, center + w2);
    }

    return range;
}

void diagram_window::handle_wheel(sf::Event::MouseWheelScrollEvent& event)
{
    float delta = event.delta * diag->get_zoom_granularity();
    float alpha = static_cast<float>(mouse_x - ZERO_X) / (ONE_X - ZERO_X);
    if (alpha < 0.0f) {
        alpha = 0.0f;
    } else if (alpha > 1.0f) {
        alpha = 1.0f;
    }

    pair<float, float> new_xrange = x_range;
    new_xrange.first += alpha * delta;
    new_xrange.second -= (1.0f - alpha) * delta;

    new_xrange = check_range(new_xrange);
    diag->set_x_range(new_xrange);
    x_range = new_xrange;
    create_x_labels();

    dirty = true;
}

void diagram_window::handle_mouse_down(sf::Event::MouseButtonEvent& event)
{
    dragging = true;
    drag_start_x = event.x;
    drag_start_y = event.y;
    drag_start_x_range = x_range;
}

void diagram_window::handle_mouse_up(sf::Event::MouseButtonEvent& event)
{
    dragging = false;

    // If the distance between the place where the key was pressed and the place
    // where it is released, we should consider this a click.
    int diff_x = event.x - drag_start_x,
        diff_y = event.y - drag_start_y;

    if (handler != nullptr &&
        event.x >= ZERO_X && event.y <= ONE_X &&
        diff_x * diff_x + diff_y * diff_y <= MAX_CLICK_DISTANCE * MAX_CLICK_DISTANCE) {

        float alpha = static_cast<float>(event.x - ZERO_X) / (ONE_X - ZERO_X),
              value = alpha * (x_range.second - x_range.first) + x_range.first;

        handler->on_click_mark(value);
    }
}

int diagram_window::get_pixels_per_drag_step()
{
    return static_cast<int>(ceil((ONE_X - ZERO_X) * diag->get_drag_step_normalized()));
}

void diagram_window::handle_mouse_move(sf::Event::MouseMoveEvent& event)
{
    if (mouse_x != event.x) {
        mouse_x = event.x;

        if (mouse_x >= ZERO_X && mouse_x <= ONE_X) {
            click_mark_x = mouse_x;
            vertical_rule.setPosition(sf::Vector2f(event.x, ONE_Y));
            dirty = true;
        }
    }

    if (!dragging) {
        return;
    }

    int new_x = event.x, diff = new_x - drag_start_x;
    float delta = -diag->get_x_granularity(min_drag_step) * (diff / get_pixels_per_drag_step());

    pair<float, float> new_xrange = drag_start_x_range;
    new_xrange.first += delta;
    new_xrange.second += delta;
    if (new_xrange.first < diag->get_full_x_range().first || new_xrange.second > diag->get_full_x_range().second) {
        return;
    }

    new_xrange = check_range(new_xrange);

    diag->set_x_range(new_xrange);
    x_range = new_xrange;
    create_x_labels();

    dirty = true;
}

void diagram_window::set_event_handler(diagram_event_handler* new_handler)
{
    handler = new_handler;
    handler->set_parent(this);
}

void diagram_window::start()
{
    while (window->isOpen())
    {
        if (dirty) {
            window->clear(sf::Color(BACK_COLOR));

            diag->draw(window.get(), make_pair(ZERO_X, ZERO_Y), make_pair(ONE_X - ZERO_X, ZERO_Y - ONE_Y));
            draw_axes();
            draw_labels();
            draw_texts();

            window->display();

            dirty = false;
        }

        sf::Event event;
        window->waitEvent(event);

        if (event.type == sf::Event::Closed) {
            window->close();
        } else if (event.type == sf::Event::MouseWheelScrolled) {
            handle_wheel(event.mouseWheelScroll);
        } else if (event.type == sf::Event::MouseButtonPressed) {
            handle_mouse_down(event.mouseButton);
        } else if (event.type == sf::Event::MouseButtonReleased)  {
            handle_mouse_up(event.mouseButton);
        } else if (event.type == sf::Event::MouseMoved) {
            handle_mouse_move(event.mouseMove);
        } else if (event.type == sf::Event::KeyReleased && handler != nullptr) {
            handler->on_key_press(event.key.code);
        }
    }
}
