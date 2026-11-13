#pragma once

#include <khepri/math/rect.hpp>
#include <khepri/utility/string.hpp>

#include <map>
#include <optional>
#include <string_view>

namespace openglyph {

/**
 * A mega texture description.
 *
 * This class is essentially an index for a mega texture: it contains a list of textures by name,
 * and their respective dimensions within the larger image.
 *
 * This class is JUST the index. The actual image data is stored separately, in a normal texture.
 */
class MegaTexture final
{
public:
    /**
     * Adds a texture to the mega texture.
     *
     * If a texture with the same name already exists, this method does nothing.
     * This method does NOT check for overlap with other textures.
     */
    void add_texture(std::string_view texture_name, const khepri::Rect& rect);

    /**
     * Returns the area in the larger texture of the specified smaller texture.
     *
     * The returned rectangle is in texels of the larger texture.
     */
    std::optional<khepri::Rect> texture_area(std::string_view texture_name) const noexcept;

private:
    std::map<std::string, khepri::Rect, khepri::CaseInsensitiveLess> m_textures;
};

} // namespace openglyph