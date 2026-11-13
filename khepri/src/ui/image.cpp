#include <khepri/ui/image.hpp>

namespace khepri::ui {

void Image::style(const Style& style) noexcept
{
    m_material_id = style.material_id;
}

void Image::image(const Renderer::TextureId texture_id, const Rect& source_rect) noexcept
{
    m_texture_id  = texture_id;
    m_source_rect = source_rect;
}

void Image::render(Renderer& renderer, const Rect& clip_rect) noexcept
{
    if (m_texture_id != Renderer::INVALID_TEXTURE) {
        Renderer::Quad quad;
        quad.area        = calculated_layout();
        quad.material_id = m_material_id;
        quad.texture_id  = m_texture_id;
        quad.tex_rect    = m_source_rect;
        renderer.render_quads({&quad, 1}, clip_rect);
    }
}

} // namespace khepri::ui
