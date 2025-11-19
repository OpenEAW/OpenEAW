#pragma once

#include "frame.hpp"
#include "label.hpp"

#include <khepri/utility/signal.hpp>

#include <functional>
#include <vector>

namespace khepri::ui {

/**
 * A checkbox.
 *
 * A checkbox has a \a checked or \a unchecked state and can be toggled between these states by
 * clicking on it.
 */
class Checkbox : public widget
{
public:
    /**
     * Checkbox states
     */
    enum class State
    {
        enabled,
        disabled,
        mouseover,
        checked,
    };

    /**
     * Properties that define a checkbox's style
     */
    struct Style
    {
        /**
         * Properties that define a checkbox's style in a state
         */
        struct State
        {
            /// Style of the checkbox's frame
            Frame::Style frame;
        };

        /// Style for the "enabled" state (when unchecked)
        State enabled;

        /// Style for the "disabled" state
        State disabled;

        /// Style for the "mouseover" state (when unchecked)
        State mouseover;

        /// Style for the "checked" state
        State checked;
    };

    /**
     * Constructs the checkbox
     * \param layout the layout for this checkbox
     */
    explicit Checkbox(const Layout& layout)
        : widget(layout), m_frame(std::make_shared<Frame>(Layout::fill()))
    {
        add(m_frame);
    }

    /**
     * Sets the checkbox's style
     *
     * \param[in] style the checkbox's style.
     */
    void style(const Style& style) noexcept
    {
        m_style = style;
        apply_active_style();
    }

    /**
     * Indicates if the checkbox is currently checked
     */
    bool checked() const noexcept
    {
        return m_checked;
    }

    /**
     * Sets the checked state of the checkbox
     *
     * \param state the new checked state
     */
    void checked(bool state) noexcept
    {
        if (m_checked != state) {
            m_checked = state;
            apply_active_style();
            m_state_changed();
        }
    }

    /**
     * Adds a state change listener to the checkbox.
     *
     * \param[in] listener the state change listener to add.
     * \return a connection object to manage the new listener connection
     *
     * State change listeners are called when the checkbox has been checked or unchecked.
     */
    auto add_state_listener(const Slot<void()>& listener)
    {
        return m_state_changed.connect(listener);
    }

    /// \see Widget::enabled()
    void enabled(bool enabled) noexcept override
    {
        widget::enabled(enabled);
        apply_active_style();
    }

protected:
    /// \see Widget::on_event()
    void on_event(const Event& e) override
    {
        if (auto* mee = std::get_if<MouseEnterEvent>(&e)) {
            m_mouse_over = true;
            apply_active_style();
        } else if (auto* mle = std::get_if<MouseLeaveEvent>(&e)) {
            m_mouse_over = false;
            apply_active_style();
        } else if (auto* mpe = std::get_if<MousePressEvent>(&e)) {
            if (mpe->button == MouseButton::left) {
                checked(!m_checked);
            }
        }
    }

private:
    void apply_active_style() noexcept
    {
        if (!Widget::enabled()) {
            apply_state_style(m_style.disabled);
        } else if (m_checked) {
            apply_state_style(m_style.checked);
        } else if (m_mouse_over) {
            apply_state_style(m_style.mouseover);
        } else {
            apply_state_style(m_style.enabled);
        }
    }

    void apply_state_style(const Style::State& state_style) noexcept
    {
        m_frame->style(state_style.frame);
    }

    Style m_style;

    khepri::Signal<void()> m_state_changed;

    bool m_checked{false};
    bool m_mouse_over{false};

    std::shared_ptr<Frame> m_frame;
};

} // namespace khepri::ui
