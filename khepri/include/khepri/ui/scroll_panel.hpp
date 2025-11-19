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
    // Container that clips the scroll panel's content
    class ClipContainer : public Widget
    {
    public:
        using Widget::Widget;

        using Widget::add;
        using Widget::clear;
        using Widget::remove;

        std::optional<Rect> clip() const noexcept override
        {
            // Clip the child widgets to this container
            auto rect = calculated_layout();
            rect.x    = 0;
            rect.y    = 0;
            return rect;
        }
    };

    // Container for all the content in the scroll panel
    class ContentContainer : public Widget
    {
    public:
        using Widget::Widget;

        using Widget::add;
        using Widget::clear;
        using Widget::remove;
    };

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
    explicit ScrollPanel(const Layout& layout) noexcept
        : Widget(layout)
        , m_scrollbar_width(layout.scrollbar_width)
        , m_frame(std::make_shared<Frame>(Frame::Layout::fill()))
        , m_clip_container(std::make_shared<ClipContainer>(ClipContainer::Layout::fill()))
        , m_content(std::make_shared<ContentContainer>(
              ContentContainer::Layout::top_left({0, 0}, {0, 0})))
        , m_scrollbar(std::make_shared<Scrollbar>(Scrollbar::Layout{
              {{1, -static_cast<long>(layout.scrollbar_width)}, {0, 0}, {1, 0}, {1, 0}},
              layout.updown_button_height,
              layout.track_button_size}))
    {
        Widget::add(m_frame);
        Widget::add(m_clip_container);
        Widget::add(m_scrollbar);

        // The ContentContainer is a convenient way to offset all content on scroll events.
        // It's actually 0x0 in size, but since content overflows, that's not a problem.
        m_clip_container->add(m_content);

        m_scrollbar->add_scroll_listener([this] { on_scroll(); });
        m_scrollbar->scroll_step(DEFAULT_SCROLL_STEP);

        resize_clip_container();
    }

    /**
     * Adds a widget as a child of this scroll panel.
     *
     * \param widget widget to add
     * \throws #khepri::ArgumentError if the widget is already added to a canvas or widget.
     *
     * This scroll panel holds a reference to the child widget as long as its added.
     */
    void add(const std::shared_ptr<Widget>& widget)
    {
        m_content->add(widget);
    }

    /**
     * Removes a widget as a child from this scroll panel.
     *
     * \param widget widget to remove
     * \throws #khepri::ArgumentError if the widget is not a child of this scroll panel
     */
    void remove(const std::shared_ptr<Widget>& widget)
    {
        m_content->remove(widget);
    }

    /**
     * Removes all child widgets.
     */
    void clear()
    {
        m_content->clear();
    }

    /**
     * Sets the scroll panel style.
     *
     * \param[in] style the scroll panel's style.
     */
    void style(const Style& style) noexcept
    {
        m_style = style;
        m_frame->style(style.frame);
        m_scrollbar->style(style.scrollbar);
        resize_clip_container();
    }

    /**
     * Returns the current scroll position.
     */
    long scroll_position() const noexcept {
        return m_scrollbar->scroll_position();
    }

protected:
    void resize_clip_container()
    {
        // Size the content panel based on the frame margins and scrollbar size
        m_clip_container->layout(
            {{0.0f, static_cast<long>(m_style.frame.margins.left)},
             {0.0f, static_cast<long>(m_style.frame.margins.top)},
             {1.0f, -static_cast<long>(m_style.frame.margins.right + m_scrollbar_width)},
             {1.0f, -static_cast<long>(m_style.frame.margins.bottom)}});
    }

    /// \see Widget::on_event()
    void on_event(const Event& e) override
    {
        if (auto* mse = std::get_if<MouseScrollEvent>(&e)) {
            // The offset is positive when the wheel scrolls "up" (for end-user UX). In this case,
            // we want the scroll position to decrement.
            m_scrollbar->scroll_position(m_scrollbar->scroll_position() -
                                         static_cast<long>(mse->offset.y) * SCROLL_MULTIPLIER);
        }
    }

    /// \see Widget::on_layout()
    void on_layout() noexcept override
    {
        // After the new layout has been updated, update our scroll range and position in case
        // the content size changed.
        const auto content_height = static_cast<long>(m_content->calculated_bounds().height);
        const auto view_height = static_cast<long>(m_clip_container->calculated_layout().height);
        m_scrollbar->scroll_range(0, std::max(content_height - view_height, 0L));
    }

private:
    // Scrolling the amount of pixels indicated by the scrollwheel feels too slow, so speed it up.
    static constexpr auto SCROLL_MULTIPLIER = 10;

    // Default increment for the scrollbar. Scroll a bunch of pixels at a time.
    static constexpr auto DEFAULT_SCROLL_STEP = 20;

    void on_scroll()
    {
        const auto scroll_pos = m_scrollbar->scroll_position();
        m_content->layout({{0.0f, 0}, {0.0f, -scroll_pos}, {0.0f, 0}, {0.0f, -scroll_pos}});
    }

    Style                             m_style;
    unsigned long                     m_scrollbar_width;
    std::shared_ptr<Frame>            m_frame;
    std::shared_ptr<ClipContainer>    m_clip_container;
    std::shared_ptr<ContentContainer> m_content;
    std::shared_ptr<Scrollbar>        m_scrollbar;
};

} // namespace khepri::ui
