#pragma once

#include <khepri/math/color_rgb.hpp>

namespace khepri::font {

/**
 * Options to create a font from a font face.
 */
struct FontOptions
{
    /// Size (em-height), in pixels
    unsigned int font_size_px{DEFAULT_FONT_SIZE_PX};

    /// Color at the top of the line's gradient
    ColorRGB color_top{1, 1, 1};

    /// Color at the bottom of the line's gradient
    ColorRGB color_bottom{1, 1, 1};

    /// Vertical scale factor for the font
    float vert_scale{1.0};

    /// Size of the font's stroke, in pixels
    float stroke_size_px{0};

    /// Color of the font's stroke (if stroke_size_px > 0)
    ColorRGB stroke_color{0, 0, 0};

    /// Is the font embossed?
    /// Embossed fonts have top borders darkened and bottom borders brightened to make it seem like
    /// they are lit from the bottom.
    bool embossed{false};

private:
    static constexpr auto DEFAULT_FONT_SIZE_PX = 13;
};

} // namespace khepri::font
