#include <khepri/ui/button.hpp>

namespace khepri::ui {

Button::Button(const Layout& layout)
    : Widget(layout)
    , m_frame(std::make_shared<Frame>(Layout::fill()))
    , m_label(std::make_shared<Label>(Layout::fill()))
{
    add(m_frame);
    add(m_label);
}

void Button::style(const Style& style) noexcept
{
    m_style = style;
    apply_active_style();
}

std::u16string_view Button::text() const noexcept
{
    return m_label->text();
}

void Button::text(std::u16string_view text)
{
    m_label->text(text);
}

void Button::clear()
{
    text({});
}

void Button::add_click_listener(const ClickListener& listener)
{
    m_click_listeners.push_back(listener);
}

void Button::enabled(bool enabled) noexcept
{
    Widget::enabled(enabled);
    apply_active_style();
}

void Button::on_event(const Event& e)
{
    if (auto* mee = std::get_if<MouseEnterEvent>(&e)) {
        m_mouse_over = true;
        apply_active_style();
    } else if (auto* mle = std::get_if<MouseLeaveEvent>(&e)) {
        m_mouse_over = false;
        apply_active_style();
    } else if (auto* mpe = std::get_if<MousePressEvent>(&e)) {
        if (mpe->button == MouseButton::left) {
            m_pressed = true;
            set_capture();
        }
        apply_active_style();
    } else if (auto* mre = std::get_if<MouseReleaseEvent>(&e)) {
        if (m_pressed) {
            m_pressed = false;
            release_capture();
            apply_active_style();
            // A click can only happen if the cursor is over the button
            if (m_mouse_over && mre->button == MouseButton::left) {
                // It's a click
                for (const auto& listener : m_click_listeners) {
                    listener();
                }
            }
        }
    }
}

void Button::apply_active_style() noexcept
{
    bool pressed_style = false;
    if (!Widget::enabled()) {
        apply_state_style(m_style.disabled);
    } else if (m_pressed) {
        apply_state_style(m_style.pressed);
        pressed_style = true;
    } else if (m_mouse_over) {
        apply_state_style(m_style.mouseover);
    } else {
        apply_state_style(m_style.enabled);
    }

    if (pressed_style) {
        label_offset(1, 1);
    } else {
        label_offset(0, 0);
    }
}

void Button::apply_state_style(const Style::State& state_style) noexcept
{
    m_frame->style(state_style.frame);
    m_label->style(state_style.label_style);
}

void Button::label_offset(long dx, long dy) noexcept
{
    m_label->layout({{0.0f, dx}, {0.0f, dy}, {1.0f, dx}, {1.0f, dy}});
}

} // namespace khepri::ui
