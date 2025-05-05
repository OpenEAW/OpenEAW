#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace khepri::font {

/**
 * \brief Description of a font face
 *
 * A font face is a collection of rendering rules for glyphs (images of characters). This
 * description can be used to create a #khepri::font::FontFace.
 */
class FontFaceDesc
{
public:
    /**
     * Constructs a font description.
     *
     * \param data the font data
     */
    explicit FontFaceDesc(std::vector<std::uint8_t> data) : m_data(std::move(data)) {}

    /**
     * Returns the font's data
     */
    [[nodiscard]] const auto& data() const noexcept
    {
        return m_data;
    }

private:
    std::vector<std::uint8_t> m_data;
};

} // namespace khepri::font
