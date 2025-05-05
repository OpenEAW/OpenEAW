#pragma once

#include "font_options.hpp"

#include <khepri/math/rect.hpp>
#include <khepri/renderer/texture_desc.hpp>

#include <string_view>

namespace khepri::font {
namespace detail {
class FontFaceState;
}

/**
 * Output of a text rendering
 *
 * This structure contains the generated texture description, and size and positioning information
 * to place the texture.
 */
struct TextRender
{
    /// The texture description with the rendered text.
    /// The texture's width and height will be a power of 2.
    khepri::renderer::TextureDesc texture_desc;

    /// The rectangle in the texture that contains the rendered text
    khepri::Rect rect{};

    /// The offset relative to \a rect.y of the text's baseline.
    int y_baseline{};
};

/**
 * \brief A font
 *
 * A font is an object that consists of a reference to a #khepri::font::FontFace and a set of
 * options that describe the font, such as size, color, etc.
 *
 * Fonts are created from a #khepri::font::FontFace.
 *
 * \see #khepri::font::FontFace::create_font
 */
class Font final
{
    friend class FontFace;

public:
    /**
     * Constructs a font
     * \param[in] face    the font face this font uses.
     * \param[in] options the specific options for this font.
     */
    Font(std::shared_ptr<detail::FontFaceState> face, const FontOptions& options);
    ~Font();

    /// Constructs a font as a copy of another font
    Font(const Font& font);

    /// Constructs a font by moving from another font
    Font(Font&& font) noexcept;

    /// Assigns a copy of another font to this object
    Font& operator=(const Font& font);

    /// Moves another font into this object
    Font& operator=(Font&& font) noexcept;

    /**
     * Renders a string.
     */
    [[nodiscard]] TextRender render(std::u16string_view text) const;

private:
    std::shared_ptr<detail::FontFaceState> m_face;
    FontOptions                            m_options;
};

} // namespace khepri::font
