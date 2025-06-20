#include "font_face_state.hpp"

#include <khepri/font/font.hpp>

namespace khepri::font {

Font::Font(std::shared_ptr<detail::FontFaceState> face, const FontOptions& options)
    : m_face(std::move(face)), m_options(options)
{
}

Font::Font(const Font& font)                = default;
Font::Font(Font&& font) noexcept            = default;
Font& Font::operator=(const Font& font)     = default;
Font& Font::operator=(Font&& font) noexcept = default;

Font::~Font() = default;

TextRender Font::render(std::u16string_view text) const
{
    return m_face->render(text, m_options);
}

} // namespace khepri::font
