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
class Checkbox : public Widget
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
    explicit Checkbox(const Layout& layout);

    /**
     * Sets the checkbox's style
     *
     * \param[in] style the checkbox's style.
     */
    void style(const Style& style) noexcept;

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
    void checked(bool state) noexcept;

    /**
     * Adds a state change listener to the checkbox.
     *
     * \param[in] listener the state change listener to add.
     * \return a connection object to manage the new listener connection
     *
     * State change listeners are called when the checkbox has been checked or unchecked.
     */
    auto add_state_listener(const Slot<void()>& listener);

    /// \see Widget::enabled()
    void enabled(bool enabled) noexcept override;

protected:
    /// \see Widget::can_focus
    bool can_focus() override
    {
        return true;
    }

    /// \see Widget::on_event()
    void on_event(const Event& e) override;

private:
    void apply_active_style() noexcept;

    void apply_state_style(const Style::State& state_style) noexcept;

    Style m_style;

    khepri::Signal<void()> m_state_changed;

    bool m_checked{false};
    bool m_mouse_over{false};

    std::shared_ptr<Frame> m_frame;
};

} // namespace khepri::ui
