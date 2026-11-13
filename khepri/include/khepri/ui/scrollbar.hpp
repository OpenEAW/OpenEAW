#pragma once

#include "button.hpp"
#include "frame.hpp"

#include <memory>
#include <string_view>
#include <tuple>

namespace khepri::ui {

/**
 * A scrollbar.
 *
 * A scrollbar is used to quickly move a "viewport" vertically through a larger content area. It
 * consists of two buttons to move the viewport up or down and a track bar with a track button to
 * allow the end-user to move to a region by using the cursor.
 */
class Scrollbar : public Widget
{
    class TrackButton;

public:
    struct Layout : public Widget::Layout
    {
        /**
         * Height of the up/down button, in canvas pixels.
         */
        unsigned long updown_button_height{0};

        /**
         * Size of the track button, in canvas pixels.
         * The track button will be centered on the track.
         */
        Size track_button_size{0, 0};
    };

    /**
     * Properties that define a scrollbar's style
     */
    struct Style
    {
        /// Style for the frame of the scrollbar
        Frame::Style frame;

        /// Style for the "up" button of the scrollbar
        Button::Style button_up;

        /// Style for the "down" button of the scrollbar
        Button::Style button_down;

        /// Style for the track area of the scrollbar when enabled
        Frame::Style frame_track_enabled;

        /// Style for the track area of the scrollbar when disabled
        Frame::Style frame_track_disabled;

        /// Style for the track button of the scrollbar
        Button::Style button_track;
    };

    /// Listener for scroll events on the scrollbar
    using ScrollListener = std::function<void()>;

    /**
     * Constructs the scrollbar
     *
     * \param layout the layout for this scrollbar
     */
    explicit Scrollbar(const Layout& layout) noexcept;

    /**
     * Sets the scroll bar style.
     *
     * \param[in] style the scroll bar's style.
     */
    void style(const Style& style) noexcept;

    /**
     * Sets the scroll range.
     * The top and bottom of the scroll track are mapped to these limits, inclusive.
     */
    void scroll_range(long min, long max) noexcept;

    /**
     * Returns the current scroll range
     */
    std::tuple<long, long> scroll_range() const noexcept
    {
        return {m_min, m_max};
    }

    /**
     * Sets the scroll increment/decrement step.
     *
     * The step is the value by which the scrollbar's position is modified every time the up or down
     * is clicked.
     */
    void scroll_step(long step) noexcept;

    /**
     * Returns the current scroll increment/decrement step.
     */
    long scroll_step() const noexcept
    {
        return m_step;
    }

    /**
     * Sets the current scroll position.
     *
     * \param position the new position. This is clamped to the current range.
     *
     * \note calling this method does not generate a change event.
     */
    void scroll_position(long position) noexcept;

    /**
     * Returns the current scroll position.
     */
    long scroll_position() const noexcept
    {
        return m_position;
    }

    /**
     * Adds a scroll listener to the scrollbar.
     *
     * \param[in] listener the click listener to add.
     *
     * Scroll listeners are called when the scrollbar's position has changed, even by someone
     * calling \ref scroll_position.
     */
    void add_scroll_listener(const ScrollListener& listener);

private:
    void update_track_button();

    Button::Layout track_button_layout(long offset_top) const noexcept;

    // Returns a (min, max) tuple for the track button's current vertical range in the track area
    // (in canvas pixels)
    std::tuple<long, long> calculated_track_range() const noexcept;

    // Called by the track button when it's dragged to another position.
    // \param cursor_position the absolute coordinate that the button is dragged to.
    void on_track_button_dragged(const MousePosition& cursor_position);

    void notify_listeners();

    Size  m_track_button_size;
    Style m_style;

    long m_min{0};
    long m_max{0};
    long m_position{0};
    long m_step{1};

    std::shared_ptr<Frame>       m_frame;
    std::shared_ptr<Button>      m_up_button;
    std::shared_ptr<Button>      m_down_button;
    std::shared_ptr<Frame>       m_track_area;
    std::shared_ptr<TrackButton> m_track_button;

    std::vector<ScrollListener> m_scroll_listeners;
};

} // namespace khepri::ui
