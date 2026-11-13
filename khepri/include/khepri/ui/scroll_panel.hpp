#pragma once

#include "align.hpp"
#include "frame.hpp"
#include "scrollbar.hpp"
#include "widget.hpp"

#include <memory>

namespace khepri::ui {

/**
 * A scrollable panel.
 *
 * A normal panel provides a grouping for widgets but overflows or cuts off child widgets that fall
 * outside of the panel. A scrollable panel cuts off child widgets but shows a vertical and/or
 * horizontal scroll bar if that happens.
 */
class ScrollPanel : public Widget
{
    class ClipContainer;
    class ContentContainer;

public:
    /**
     * Properties that define a scroll panel's style
     */
    struct Style
    {
        /// Style for the frame of the scroll panel
        Frame::Style frame;

        /// Style for the scrollbar of the scroll panel
        Scrollbar::Style scrollbar;
    };

    struct Layout : public Widget::Layout
    {
        /**
         * Width of the scrollbar, in canvas pixels.
         */
        unsigned long scrollbar_width;

        /**
         * Height of the up/down button, in canvas pixels.
         */
        unsigned long updown_button_height;

        /**
         * Size of the track button, in canvas pixels.
         * The track button will be centered on the track.
         */
        Size track_button_size;
    };

    /**
     * Constructs the scroll_panel
     *
     * \param layout the layout for this listbox
     */
    explicit ScrollPanel(const Layout& layout) noexcept;

    /**
     * Adds a widget as a child of this scroll panel.
     *
     * \param widget widget to add
     * \throws #khepri::ArgumentError if the widget is already added to a canvas or widget.
     *
     * This scroll panel holds a reference to the child widget as long as its added.
     */
    void add(const std::shared_ptr<Widget>& widget);

    /**
     * Removes a widget as a child from this scroll panel.
     *
     * \param widget widget to remove
     * \throws #khepri::ArgumentError if the widget is not a child of this scroll panel
     */
    void remove(const std::shared_ptr<Widget>& widget);

    /**
     * Removes all child widgets.
     */
    void clear();

    /**
     * Sets the scroll panel style.
     *
     * \param[in] style the scroll panel's style.
     */
    void style(const Style& style) noexcept;

    /**
     * Returns the current scroll position.
     */
    long scroll_position() const noexcept;

    /**
     * Returns the bounding box of the content area of this scroll panel.
     *
     * \note This is not the size of the possibly-overflowing content, but rather the area in which
     * content is visible.
     */
    const auto& content_bounds() const noexcept
    {
        return m_content_bounds;
    }

protected:
    void resize_clip_container();

    /// \see Widget::on_event()
    void on_event(const Event& e) override;

    /// \see Widget::on_layout()
    void on_layout() noexcept override;

private:
    void on_scroll();

    Style                             m_style;
    unsigned long                     m_scrollbar_width;
    std::shared_ptr<Frame>            m_frame;
    std::shared_ptr<ClipContainer>    m_clip_container;
    std::shared_ptr<ContentContainer> m_content;
    std::shared_ptr<Scrollbar>        m_scrollbar;
    khepri::Rect                      m_content_bounds;
};

} // namespace khepri::ui
