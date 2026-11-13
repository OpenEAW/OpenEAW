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
        /// The label's material
        Renderer::MaterialId material{Renderer::INVALID_MATERIAL};

        /// The label's font
        Renderer::FontId font{Renderer::INVALID_FONT};

        /// The label's text alignment
        TextAlign text_align{TextAlign::left};
    };

    using Widget::Widget;

    /// \see #khepri::ui::Widget::render
    void render(Renderer& renderer, const Rect& clip_rect) noexcept override;

    /**
     * Sets the label's style
     *
     * \param[in] style the label's style.
     */
    void style(const Style& style) noexcept;

    /**
     * Retrieves the label's text
     */
    std::u16string_view text() const noexcept
    {
        return m_text.string;
    }

    /**
     * Sets the label's text
     *
     * \param[in] text the new text
     */
    void text(std::u16string_view text);

    /**
     * Clears the label's text
     */
    void clear();

private:
    Renderer::Text m_text;
};

} // namespace khepri::ui
