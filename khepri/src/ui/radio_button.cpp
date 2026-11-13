#include <khepri/ui/radio_button.hpp>

namespace khepri::ui {

RadioButton::RadioButton(const Layout& layout)
    : Widget(layout), m_frame(std::make_shared<Frame>(Layout::fill()))
{
    add(m_frame);
}

void RadioButton::style(const Style& style) noexcept
{
    m_style = style;
    apply_active_style();
}

void RadioButton::selected(bool state) noexcept
{
    if (m_selected != state) {
        m_selected = state;
        apply_active_style();
        m_state_changed();
    }
}

Connection RadioButton::add_state_listener(const Slot<void()>& listener)
{
    return m_state_changed.connect(listener);
}

void RadioButton::enabled(bool enabled) noexcept
{
    Widget::enabled(enabled);
    apply_active_style();
}

void RadioButton::on_event(const Event& e)
{
    if (auto* mee = std::get_if<MouseEnterEvent>(&e)) {
        m_mouse_over = true;
        apply_active_style();
    } else if (auto* mle = std::get_if<MouseLeaveEvent>(&e)) {
        m_mouse_over = false;
        apply_active_style();
    } else if (auto* mpe = std::get_if<MousePressEvent>(&e)) {
        if (mpe->button == MouseButton::left) {
            selected(true);
        }
    }
}

void RadioButton::apply_active_style() noexcept
{
    if (!Widget::enabled()) {
        apply_state_style(m_style.disabled);
    } else if (m_selected) {
        apply_state_style(m_style.selected);
    } else if (m_mouse_over) {
        apply_state_style(m_style.mouseover);
    } else {
        apply_state_style(m_style.enabled);
    }
}

void RadioButton::apply_state_style(const Style::State& state_style) noexcept
{
    m_frame->style(state_style.frame);
}

} // namespace khepri::ui
