#pragma once

#include "widget.hpp"

#include <khepri/math/color_rgba.hpp>

namespace khepri::ui {

/**
 * A widget that's just a simple rectangle
 */
class Rectangle : public Widget
{
public:
    /**
     * Properties that define a rectangle's style
     */
    struct Style
    {
        /// The rectangle's material
        Renderer::MaterialId material{Renderer::INVALID_MATERIAL};

        /// The rectangle's color
        khepri::ColorRGBA color{1, 1, 1, 1};
    };

    /**
     * Constructs the rectangle
     *
     * \param layout the layout for this rectangle
     */
    explicit Rectangle(const Layout& layout) noexcept;

    /// \see #khepri::ui::Widget::render
    void render(Renderer& renderer, const Rect& clip_rect) noexcept override;

    /**
     * Sets the rectangle's style
     *
     * \param[in] style the rectangle's style.
     */
    void style(const Style& style) noexcept;

private:
    void on_layout() noexcept override;

    Renderer::FilledRect m_rect;
};

} // namespace khepri::ui
