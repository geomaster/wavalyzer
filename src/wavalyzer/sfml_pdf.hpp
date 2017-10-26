#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include <exception>
#include <hpdf.h>

namespace wavalyzer::gui {
    class diagram;

    class sfml_pdf_exception : public std::exception {
    private:
        std::string message;

    public:
        sfml_pdf_exception(const std::string& _message) : message(_message) {}
        virtual const char* what() const throw() { return message.c_str(); }
    };

    enum text_alignment {
        TEXT_ALIGN_LEFT,
        TEXT_ALIGN_RIGHT,
        TEXT_ALIGN_CENTER
    };

    class sfml_pdf {
    private:
        HPDF_Doc pdf;
        HPDF_Font font;
        HPDF_Page page;

        double draw_offset_x, draw_offset_y;

        void check_return_status(HPDF_STATUS status);

        void draw_line(double weight, int x1, int y1, int x2, int y2);
        void draw_text(int size, int x, int y, const std::string& text, text_alignment alignment = TEXT_ALIGN_CENTER);
    public:
        sfml_pdf();

        void draw(const sf::RectangleShape& rectangle);
        void draw(uint8_t *pixels, std::pair<int, int> position, std::pair<int, int> size);

        void draw_diagram(diagram* diag, bool dark = false);

        void save_to_file(const std::string& filename);

        ~sfml_pdf();
    };
}
