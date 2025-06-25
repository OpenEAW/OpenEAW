#pragma once

#include "font.hpp"
#include "font_face_desc.hpp"
#include "font_options.hpp"

#include <memory>

namespace khepri::font {
namespace detail {
class FontFaceState;
}

/**
 * A font face.
 *
 * A font face generically describes a font. It can be instantiated into individial fonts, each with
 * their own size, color, outline, and so on.
 */
class FontFace final
{
public:
    /**
     * Constructs the font face
     */
    FontFace(const FontFaceDesc& font_face_desc);
    FontFace(const FontFace&) = default;
    FontFace(FontFace&&)      = default;
    ~FontFace();

    FontFace& operator=(const FontFace&) = default;
    FontFace& operator=(FontFace&&)      = default;

    /**
     * Creates a font.
     *
     * \param[in] font_options the font options used to create the font from this face.
     *
     * A #khepri::font::Font is an object that represents all font options to render a font face
     * with.
     *
     * \return the created font
     */
    std::unique_ptr<Font> create_font(const FontOptions& font_options);

private:
    std::shared_ptr<detail::FontFaceState> m_state;
};

} // namespace khepri::font
