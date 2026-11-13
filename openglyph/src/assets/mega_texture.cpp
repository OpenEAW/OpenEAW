#include <openglyph/assets/mega_texture.hpp>

namespace openglyph {

void MegaTexture::add_texture(std::string_view texture_name, const khepri::Rect& rect)
{
    m_textures.emplace(texture_name, rect);
}

std::optional<khepri::Rect> MegaTexture::texture_area(std::string_view texture_name) const noexcept
{
    if (const auto it = m_textures.find(texture_name); it != m_textures.end()) {
        return it->second;
    }
    return {};
}

} // namespace openglyph