#include "sfml_pdf.hpp"
#include "gui.hpp"
#include <hpdf.h>
#include <iostream>
#include <cstdlib>

using namespace std;
using namespace wavalyzer::gui;
using namespace wavalyzer;

const double MARGIN = 50.0;
const double LABEL_OFFSET = 10.0;
const double PAGE_WIDTH = 841.89;
const double PAGE_HEIGHT = 595.276;
const double LABEL_AXIS_GAP = 10.0;

const double AXIS_LINE_WEIGHT = 1.0;
const double ETCH_LINE_WEIGHT = 0.5;
const double ETCH_EXTENT = 5.0;

const double GUIDELINE_WEIGHT = 0.25;

const double X_LABEL_FONT_SIZE = 10.0;
const double Y_LABEL_FONT_SIZE = 10.0;

const double OVERSAMPLE_FACTOR = 2.0;


void error_handler(HPDF_STATUS error_no, HPDF_STATUS, void*)
{
    cerr << "FATAL: libharu error: (" << error_no << ", " << ")" << endl <<
            "Unsafe to proceed (C interop shenanigans). Bailing." << endl;
}

void sfml_pdf::check_return_status(HPDF_STATUS status)
{
    if (status != HPDF_OK) {
        throw sfml_pdf_exception("libharu: error " + to_string(status));
    }
}
sfml_pdf::sfml_pdf()
{
    pdf = HPDF_New(error_handler, nullptr);
    if (!pdf) {
        throw sfml_pdf_exception("Cannot create PDF context");
    }

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    font = HPDF_GetFont(pdf, "Courier", nullptr);
    page = HPDF_AddPage(pdf);

    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_LANDSCAPE);
}

void sfml_pdf::draw_line(double weight, int x1, int y1, int x2, int y2)
{
    HPDF_Page_SetLineWidth(page, weight);
    HPDF_Page_MoveTo(page, x1, y1);
    HPDF_Page_LineTo(page, x2, y2);
    HPDF_Page_Stroke(page);
}

void sfml_pdf::draw_text(int size, int x, int y, const string& text, text_alignment alignment)
{
    HPDF_Page_SetFontAndSize(page, font, size);
    double tw = HPDF_Page_TextWidth(page, text.c_str());

    double factor;
    switch (alignment) {
        case TEXT_ALIGN_LEFT: factor = 0.0; break;
        case TEXT_ALIGN_CENTER: factor = 0.5; break;
        case TEXT_ALIGN_RIGHT: factor = 1.0; break;
        default: factor = 0.0;
    }

    HPDF_Page_BeginText(page);
    HPDF_Page_MoveTextPos(page, x - tw * factor, y - size / 2.0);
    HPDF_Page_ShowText(page, text.c_str());
    HPDF_Page_EndText(page);
}

void sfml_pdf::save_to_file(const std::string& filename)
{
    check_return_status(HPDF_SaveToFile(pdf, filename.c_str()));
}

void sfml_pdf::draw(const sf::RectangleShape& rectangle)
{
    // Get the top left corner
    sf::Vector2f top_left = rectangle.getPosition();
    sf::Vector2f size = rectangle.getSize();

    // Transform back
    top_left.x /= OVERSAMPLE_FACTOR;
    top_left.x += draw_offset_x;

    top_left.y += size.y;
    top_left.y /= OVERSAMPLE_FACTOR;
    top_left.y *= -1.0f;
    top_left.y += draw_offset_y;

    size.x /= OVERSAMPLE_FACTOR;
    size.y /= OVERSAMPLE_FACTOR;

    HPDF_Page_Rectangle(page, top_left.x, top_left.y, size.x, size.y);
    HPDF_Page_Fill(page);
}

void sfml_pdf::draw(uint8_t *pixels, pair<int, int> position, pair<int, int> size)
{
    double x = position.first, y = position.second, w = size.first, h = size.second;

    // Transform back
    x /= OVERSAMPLE_FACTOR;
    x += draw_offset_x;

    y += h;
    y /= OVERSAMPLE_FACTOR;
    y *= -1.0f;
    y += draw_offset_y;

    w /= OVERSAMPLE_FACTOR;
    h /= OVERSAMPLE_FACTOR;

    // Create a size more suitable for libharu
    vector<uint8_t> rgb_pixels(size.first * size.second * 3);
    for (size_t y = 0; y < size.second; y++) {
        for (size_t x = 0; x < size.first; x++) {
            rgb_pixels[y * size.first * 3 + x * 3 + 0] = pixels[y * size.first * 4 + x * 4 + 0];
            rgb_pixels[y * size.first * 3 + x * 3 + 1] = pixels[y * size.first * 4 + x * 4 + 1];
            rgb_pixels[y * size.first * 3 + x * 3 + 2] = pixels[y * size.first * 4 + x * 4 + 2];
        }
    }

    HPDF_Image image = HPDF_LoadRawImageFromMem(pdf,
                                                &rgb_pixels[0],
                                                size.first,
                                                size.second,
                                                HPDF_CS_DEVICE_RGB,
                                                8);

    HPDF_Page_DrawImage(page, image, x, y, w, h);
}

void sfml_pdf::draw_diagram(diagram* diag, bool dark)
{
    double zero_x = MARGIN + LABEL_OFFSET,
        zero_y = MARGIN + LABEL_OFFSET,
        one_x = PAGE_WIDTH - MARGIN,
        one_y = PAGE_HEIGHT - MARGIN;

    // Draw the main axes
    draw_line(AXIS_LINE_WEIGHT, zero_x, zero_y, zero_x, one_y);
    draw_line(AXIS_LINE_WEIGHT, zero_x, zero_y, one_x, zero_y);

    // Render the actual diagram
    draw_offset_x = zero_x;
    draw_offset_y = zero_y;
    diag->draw_to_pdf(*this, make_pair(0, 0),
            make_pair((one_x - zero_x) * OVERSAMPLE_FACTOR, (one_y - zero_y) * OVERSAMPLE_FACTOR));

    // Get the X labels and draw them
    map<float, string> x_labels = diag->get_x_labels();
    double x_label_y = MARGIN + LABEL_OFFSET - LABEL_AXIS_GAP;

    for (const auto& label : x_labels) {
        double x = label.first * (one_x - zero_x) + zero_x;
        draw_text(X_LABEL_FONT_SIZE, x, x_label_y, label.second);

        if (dark) {
            HPDF_Page_SetRGBStroke(page, 1.0, 1.0, 1.0);
        } else {
            HPDF_Page_SetRGBStroke(page, 0.0, 0.0, 0.0);
        }

        draw_line(ETCH_LINE_WEIGHT, x, zero_y - ETCH_EXTENT, x, zero_y + ETCH_EXTENT);
    }

    // Likewise for Y labels
    map<float, string> y_labels = diag->get_y_labels();
    double y_label_x = zero_x - LABEL_AXIS_GAP;

    for (const auto& label : y_labels) {
        double y = zero_y - label.first * (zero_y - one_y);
        draw_text(Y_LABEL_FONT_SIZE, y_label_x, y, label.second, TEXT_ALIGN_RIGHT);

        if (dark) {
            HPDF_Page_SetRGBStroke(page, 1.0, 1.0, 1.0);
        } else {
            HPDF_Page_SetRGBStroke(page, 0.0, 0.0, 0.0);
        }

        draw_line(GUIDELINE_WEIGHT, zero_x, y, one_x, y);
    }
}

sfml_pdf::~sfml_pdf()
{
    HPDF_Free(pdf);
}
