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
    class TrackButton : public Button
    {
    public:
        TrackButton(Scrollbar& scrollbar, const Layout& layout)
            : Button(layout), m_scrollbar(scrollbar)
        {}

    protected:
        /// \see Widget::on_event()
        void on_event(const Event& e) override
        {
            // Intercept mouse press and release events; we don't want normal button "click"
            // behavior, we want to track the button when pressed.
            if (auto* mpe = std::get_if<MousePressEvent>(&e)) {
                if (mpe->button == MouseButton::left) {
                    m_pressed = true;
                    set_capture();
                }
            } else if (auto* mre = std::get_if<MouseReleaseEvent>(&e)) {
                m_pressed = false;
                release_capture();
            } else if (auto* mme = std::get_if<MouseMoveEvent>(&e)) {
                if (m_pressed) {
                    m_scrollbar.on_track_button_dragged(mme->cursor_position);
                }
            }

            Button::on_event(e);
        }

    private:
        Scrollbar& m_scrollbar;
        bool       m_pressed{false};
    };

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

        /// Style for the "up" botton of the scrollbar
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
    explicit Scrollbar(const Layout& layout) noexcept
        : Widget(layout)
        , m_track_button_size(layout.track_button_size)
        , m_frame(std::make_shared<Frame>(Frame::Layout::fill()))
        , m_up_button(std::make_shared<Button>(Button::Layout{
              {0, 0}, {0, 0}, {1, 0}, {0, static_cast<long>(layout.updown_button_height)}}))
        , m_down_button(std::make_shared<Button>(Button::Layout{
              {0, 0}, {1, -static_cast<long>(layout.updown_button_height)}, {1, 0}, {1, 0}}))
        , m_track_area(std::make_shared<Frame>(
              Frame::Layout{{0, 0},
                            {0, static_cast<long>(layout.updown_button_height)},
                            {1, 0},
                            {1, -static_cast<long>(layout.updown_button_height)}}))
        , m_track_button(std::make_shared<TrackButton>(*this, track_button_layout(0)))
    {
        add(m_frame);
        m_frame->add(m_up_button);
        m_frame->add(m_down_button);
        m_frame->add(m_track_area);
        m_track_area->add(m_track_button);

        m_up_button->add_click_listener([this] { scroll_position(m_position - m_step); });
        m_down_button->add_click_listener([this] { scroll_position(m_position + m_step); });

        update_track_button();
    }

    /**
     * Sets the scroll bar style.
     *
     * \param[in] style the scroll bar's style.
     */
    void style(const Style& style) noexcept
    {
        m_style = style;
        m_frame->style(style.frame);
        m_up_button->style(style.button_up);
        m_down_button->style(style.button_down);
        m_track_area->style(style.frame_track_enabled);
        m_track_button->style(style.button_track);
    }

    /**
     * Sets the scroll range.
     * The top and bottom of the scroll track are mapped to these limits, inclusive.
     */
    void scroll_range(long min, long max) noexcept
    {
        m_min = min;
        m_max = max;

        // Re-set the position to clamp it to the new range and notify if it changed.
        scroll_position(m_position);

        update_track_button();
    }

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
    void scroll_step(long step) noexcept
    {
        m_step = step;
    }

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
    void scroll_position(long position) noexcept
    {
        position = clamp(position, m_min, m_max);
        if (m_position != position) {
            m_position = position;
            update_track_button();
            notify_listeners();
        }
    }

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
    void add_scroll_listener(const ScrollListener& listener)
    {
        m_scroll_listeners.push_back(listener);
    }

private:
    void update_track_button()
    {
        assert(m_position >= m_min);
        assert(m_position <= m_max);

        bool scroll_enabled = m_min != m_max;
        m_track_button->enabled(scroll_enabled);
        m_up_button->enabled(scroll_enabled);
        m_down_button->enabled(scroll_enabled);
        m_track_area->style(scroll_enabled ? m_style.frame_track_enabled
                                           : m_style.frame_track_disabled);

        // Convert to fraction along track (0 - 1).
        const auto track_frac =
            static_cast<float>(m_position - m_min) / static_cast<float>(m_max - m_min);

        // Convert to offset in range, in pixels
        const auto [track_min, track_max] = calculated_track_range();
        const auto ofs_top =
            static_cast<long>(static_cast<float>(track_max - track_min) * track_frac);

        m_track_button->layout(track_button_layout(ofs_top));
    }

    TrackButton::Layout track_button_layout(long offset_top) const noexcept
    {
        const auto ofs_left = -static_cast<long>(m_track_button_size.width / 2);

        return {{0.5f, ofs_left},
                {0.0f, offset_top},
                {0.5f, ofs_left + static_cast<long>(m_track_button_size.width)},
                {0.0f, offset_top + static_cast<long>(m_track_button_size.height)}};
    }

    // Returns a (min, max) tuple for the track button's current vertical range in the track area
    // (in canvas pixels)
    std::tuple<long, long> calculated_track_range() const noexcept
    {
        // This is basically the track area height minus half the button height so that
        // center of the button can move from the min to the max.
        const auto& area           = m_track_area->calculated_layout();
        const auto  btn_height     = static_cast<long>(m_track_button->calculated_layout().height);
        const auto  btn_top_margin = btn_height / 2;
        const auto  btn_bottom_margin = btn_height - btn_height / 2;
        const auto  track_y_start     = area.y + btn_top_margin;
        const auto  track_y_end       = area.y + area.height - btn_bottom_margin;
        return {track_y_start, track_y_end};
    }

    // Called by the track button when it's dragged to another position.
    // \param cursor_position the absolute coordinate that the button is dragged to.
    void on_track_button_dragged(const Point& cursor_position)
    {
        // Clamp the position between the top and bottom of the track area
        const auto [track_min, track_max] = calculated_track_range();
        const auto track_offset           = clamp(cursor_position.y, track_min, track_max);

        // Convert to fraction along the track (0 - 1).
        const float track_frac =
            static_cast<float>(track_offset - track_min) / (track_max - track_min);

        // Convert to track value in scrollbar range
        const long track_value = static_cast<long>(
            lerp(static_cast<float>(m_min), static_cast<float>(m_max), track_frac));

        // Set the track position
        scroll_position(track_value);
    }

    void notify_listeners()
    {
        for (auto& listener : m_scroll_listeners) {
            listener();
        }
    }

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
