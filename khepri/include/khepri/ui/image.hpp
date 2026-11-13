#pragma once

#include "frame.hpp"
#include "widget.hpp"

namespace khepri::ui {

/**
 * A widget that can display an image
 */
class Image final : public Widget
{
public:
    struct Style
    {
        /// The material to use for rendering the image.
        Renderer::MaterialId material_id{Renderer::INVALID_MATERIAL};
        Frame::Style         frame;
    };

    using Widget::Widget;

    void style(const Style& style) noexcept;

    /**
     * Sets the image to display.
     * \param[in] texture_id the texture to use for the image.
     * \param[in] source_rect the area of the texture to use for the image, in texels.
     */
    void image(const Renderer::TextureId texture_id, const Rect& source_rect) noexcept;

private:
    void render(Renderer& renderer, const Rect& clip_rect) noexcept override;

    Renderer::TextureId  m_texture_id{Renderer::INVALID_TEXTURE};
    Rect                 m_source_rect{0, 0, 0, 0};
    Renderer::MaterialId m_material_id{Renderer::INVALID_MATERIAL};
};

} // namespace khepri::ui
