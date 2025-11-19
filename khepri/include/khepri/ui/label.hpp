#pragma once

#include "align.hpp"
#include "widget.hpp"

#include <khepri/font/font.hpp>

#include <memory>
#include <string_view>

namespace khepri::ui {

/**
 * A widget with rendered text
 */
class Label : public Widget
{
public:
    /**
     * Properties that define a label's style
     */
    struct Style
    {
        /// The label's font
        std::shared_ptr<khepri::font::Font> font;

        /// The label's text alignment
        TextAlign text_align{TextAlign::center};
    };

    /**
     * Constructs the label
     *
     * \param layout the layout for this label
     */
    explicit Label(const Layout& layout) noexcept : Widget(layout)
    {}

    /// \see #khepri::ui::Widget::render
    gsl::span<const Quad> render(khepri::renderer::Renderer& renderer) noexcept override
    {
        if (m_texture == nullptr) {
            initialize_quad(renderer);
        }

        // Layout the quad to be centered
        const auto rect = calculated_layout();
        long       dx   = 0;
        switch (m_style.text_align) {
        case TextAlign::left:
        default:
            dx = 0;
            break;
        case TextAlign::center:
            dx = (static_cast<long>(rect.width) - static_cast<long>(m_quad.tex_rect.width)) / 2;
            break;
        case TextAlign::right:
            dx = static_cast<long>(rect.width) - static_cast<long>(m_quad.tex_rect.width);
            break;
        }
        // Vertically center
        long dy = (static_cast<long>(rect.height) - static_cast<long>(m_quad.tex_rect.height)) / 2;

        m_quad.area.x = rect.x + dx;
        m_quad.area.y = rect.y + dy;

        return {&m_quad, 1};
    }

    /**
     * Sets the label's style
     *
     * \param[in] style the label's style.
     */
    void style(const Style& style) noexcept
    {
        m_style = style;
        invalidate();
    }

    /**
     * Retrieves the label's text
     */
    const auto& text() const noexcept
    {
        return m_text;
    }

    /**
     * Sets the label's text
     *
     * \param[in] text the new text
     */
    void text(const std::u16string& text)
    {
        if (m_text != text) {
            m_text = text;
            invalidate();
        }
    }

private:
    void invalidate()
    {
        // Reset the texture so it gets regenerated
        m_texture = nullptr;
    }

    void initialize_quad(khepri::renderer::Renderer& renderer)
    {
        assert(m_texture == nullptr);
        if (m_style.font) {
            const auto text_render = m_style.font->render(m_text);

            m_texture = renderer.create_texture(text_render.texture_desc);

            m_quad.texture  = m_texture.get();
            m_quad.tex_rect = text_render.rect;
            m_quad.area     = {0, 0, text_render.rect.width, text_render.rect.height};
        } else {
            m_quad = {{0, 0, 0, 0}, nullptr, {0, 0, 0, 0}};
        }
    }

    Style          m_style;
    std::u16string m_text;

    std::unique_ptr<khepri::renderer::Texture> m_texture;
    Quad                                       m_quad;
};

} // namespace khepri::ui
