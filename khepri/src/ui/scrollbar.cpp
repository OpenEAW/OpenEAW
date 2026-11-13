#include <khepri/ui/scrollbar.hpp>

namespace khepri::ui {

class Scrollbar::TrackButton : public Button
{
public:
    TrackButton(Scrollbar& scrollbar, const Layout& layout) : Button(layout), m_scrollbar(scrollbar)
    {
    }

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
                m_scrollbar.on_track_button_dragged(mme->position);
            }
        }

        Button::on_event(e);
    }

private:
    Scrollbar& m_scrollbar;
    bool       m_pressed{false};
};

Scrollbar::Scrollbar(const Layout& layout) noexcept
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

void Scrollbar::style(const Style& style) noexcept
{
    m_style = style;
    m_frame->style(style.frame);
    m_up_button->style(style.button_up);
    m_down_button->style(style.button_down);
    m_track_area->style(style.frame_track_enabled);
    m_track_button->style(style.button_track);
}

void Scrollbar::scroll_range(long min, long max) noexcept
{
    m_min = min;
    m_max = max;

    // Re-set the position to clamp it to the new range and notify if it changed.
    scroll_position(m_position);

    update_track_button();
}

void Scrollbar::scroll_step(long step) noexcept
{
    m_step = step;
}

void Scrollbar::scroll_position(long position) noexcept
{
    position = clamp(position, m_min, m_max);
    if (m_position != position) {
        m_position = position;
        update_track_button();
        notify_listeners();
    }
}

void Scrollbar::add_scroll_listener(const ScrollListener& listener)
{
    m_scroll_listeners.push_back(listener);
}

void Scrollbar::update_track_button()
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
    const auto ofs_top = static_cast<long>(static_cast<float>(track_max - track_min) * track_frac);

    m_track_button->layout(track_button_layout(ofs_top));
}

Button::Layout Scrollbar::track_button_layout(long offset_top) const noexcept
{
    const auto ofs_left = -static_cast<long>(m_track_button_size.width / 2);

    return {{0.5f, ofs_left},
            {0.0f, offset_top},
            {0.5f, ofs_left + static_cast<long>(m_track_button_size.width)},
            {0.0f, offset_top + static_cast<long>(m_track_button_size.height)}};
}

// Returns a (min, max) tuple for the track button's current vertical range in the track area
// (in canvas pixels)
std::tuple<long, long> Scrollbar::calculated_track_range() const noexcept
{
    // This is basically the track area height minus half the button height so that
    // center of the button can move from the min to the max.
    const auto& area              = m_track_area->calculated_layout();
    const auto  btn_height        = static_cast<long>(m_track_button->calculated_layout().height);
    const auto  btn_top_margin    = btn_height / 2;
    const auto  btn_bottom_margin = btn_height - btn_height / 2;
    const auto  track_y_start     = area.y + btn_top_margin;
    const auto  track_y_end       = area.y + area.height - btn_bottom_margin;
    return {track_y_start, track_y_end};
}

// Called by the track button when it's dragged to another position.
// \param cursor_position the absolute coordinate that the button is dragged to.
void Scrollbar::on_track_button_dragged(const MousePosition& cursor_position)
{
    // Clamp the position between the top and bottom of the track area
    const auto [track_min, track_max] = calculated_track_range();
    const auto track_offset           = clamp(cursor_position.y, track_min, track_max);

    // Convert to fraction along the track (0 - 1).
    const float track_frac = static_cast<float>(track_offset - track_min) / (track_max - track_min);

    // Convert to track value in scrollbar range
    const long track_value =
        static_cast<long>(lerp(static_cast<float>(m_min), static_cast<float>(m_max), track_frac));

    // Set the track position
    scroll_position(track_value);
}

void Scrollbar::notify_listeners()
{
    for (auto& listener : m_scroll_listeners) {
        listener();
    }
}

} // namespace khepri::ui
