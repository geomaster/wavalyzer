#include "gui.hpp"
#include <SFML/Graphics.hpp>

using namespace wavalyzer::gui;
using namespace std;

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 600;

const int ZERO_Y = 545;
const int ONE_Y = 35;

const int ZERO_X = 100;
const int ONE_X = 1175;

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

diagram_window::diagram_window(diagram* _diagram) : diag(_diagram)
{
    window = make_unique<sf::RenderWindow>(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "wavalyzer gui");
    if (!font.loadFromFile("assets/FiraMono-Regular.otf")) {
        throw gui_exception("Cannot load font");
    }

    diag->set_x_range(diag->get_full_x_range());

    create_x_labels();
    create_y_labels();
    create_axes();
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

void diagram_window::create_axes()
{
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

void diagram_window::start()
{
    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window->close();
        }

        window->clear(sf::Color(BACK_COLOR));

        draw_axes();
        draw_labels();
        diag->draw(window.get(), make_pair(ZERO_X, ZERO_Y), make_pair(ONE_X - ZERO_X, ZERO_Y - ONE_Y));

        window->display();
    }
}
