#include "font_face_state.hpp"

#include <khepri/font/font_face.hpp>

namespace khepri::font {

FontFace::FontFace(const FontFaceDesc& font_face_desc)
    : m_state(std::make_shared<detail::FontFaceState>(font_face_desc))
{}

FontFace::~FontFace() = default;

std::unique_ptr<khepri::font::Font> FontFace::create_font(const FontOptions& font_options)
{
    return std::make_unique<khepri::font::Font>(m_state, font_options);
}

} // namespace khepri::font
