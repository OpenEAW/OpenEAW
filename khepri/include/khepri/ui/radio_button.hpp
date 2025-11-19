#pragma once

#include "frame.hpp"
#include "label.hpp"

#include <khepri/utility/signal.hpp>

#include <functional>
#include <vector>

namespace khepri::ui {

/**
 * A radio button.
 *
 * A radio button is like a \ref Checkbox : selected or deselected, but is also tied to a \ref
 * RadioGroup, which ensures only one of the radio buttons in that group is selected.
 */
class RadioButton : public Widget
{
public:
    /**
     * Radio button states
     */
    enum class State
    {
        enabled,
        disabled,
        mouseover,
        selected,
    };

    /**
     * Properties that define a radio button's style
     */
    struct Style
    {
        /**
         * Properties that define a radio button's style in a state
         */
        struct State
        {
            /// Style of the radio button's frame
            Frame::Style frame;
        };

        /// Style for the "enabled" state (when unchecked)
        State enabled;

        /// Style for the "disabled" state
        State disabled;

        /// Style for the "mouseover" state (when unchecked)
        State mouseover;

        /// Style for the "selected" state
        State selected;
    };

    /**
     * Constructs the radio button
     * \param layout the layout for this radio button
     */
    explicit RadioButton(const Layout& layout)
        : Widget(layout), m_frame(std::make_shared<Frame>(Layout::fill()))
    {
        add(m_frame);
    }

    /**
     * Sets the radio button's style
     *
     * \param[in] style the radio button's style.
     */
    void style(const Style& style) noexcept
    {
        m_style = style;
        apply_active_style();
    }

    /**
     * Indicates if the button is currently selected
     */
    bool selected() const noexcept
    {
        return m_selected;
    }

    /**
     * Sets the selected state of the button
     *
     * \param state the new selected state
     *
     * This updates this button only. To update all radio buttons in a \ref RadioGroup to reflect a
     * logical selection, use \ref RadioGroup::select.
     */
    void selected(bool state) noexcept
    {
        if (m_selected != state) {
            m_selected = state;
            apply_active_style();
            m_state_changed();
        }
    }

    /**
     * Adds a state change listener to the button.
     *
     * \param[in] listener the state change listener to add.
     * \return a connection object to manage the new listener connection
     *
     * State change listeners are called when the radio button has been selected or deselected.
     * These listeners apply only this button. To get the state of the entire radio group, use \ref
     * RadioGroup::add_selection_listener.
     */
    auto add_state_listener(const Slot<void()>& listener)
    {
        return m_state_changed.connect(listener);
    }

    /// \see widget::enabled()
    void enabled(bool enabled) noexcept override
    {
        Widget::enabled(enabled);
        apply_active_style();
    }

protected:
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
                selected(true);
            }
        }
    }

private:
    void apply_active_style() noexcept
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

    void apply_state_style(const Style::State& state_style) noexcept
    {
        m_frame->style(state_style.frame);
    }

    Style m_style;

    khepri::Signal<void()> m_state_changed;

    bool m_selected{false};
    bool m_mouse_over{false};

    std::shared_ptr<Frame> m_frame;
};

} // namespace khepri::ui
