#pragma once

#include "frame.hpp"
#include "label.hpp"

#include <functional>
#include <vector>

namespace khepri::ui {

/**
 * An interactive UI button
 */
class Button : public Widget
{
public:
    /**
     * Button states
     */
    enum class State
    {
        enabled,
        disabled,
        mouseover,
        pressed,
    };

    /**
     * Properties that define a button's style
     */
    struct Style
    {
        /**
         * Properties that define a button's style in a state
         */
        struct State
        {
            /// Style of the button's frame
            Frame::Style frame;

            /// The button's label's style
            Label::Style label_style;
        };

        /// Style for the "enabled" state
        State enabled;

        /// Style for the "disabled" state
        State disabled;

        /// Style for the "mouseover" state
        State mouseover;

        /// Style for the "pressed" state
        State pressed;
    };

    /// Listener for click events on the button
    using ClickListener = std::function<void()>;

    /**
     * Constructs the button
     * \param layout the layout for this button
     */
    explicit Button(const Layout& layout);

    /**
     * Sets the button's style
     *
     * \param[in] style the button's style.
     */
    void style(const Style& style) noexcept;

    /**
     * Retrieves the button's text
     */
    std::u16string_view text() const noexcept;

    /**
     * Sets the button's text
     *
     * \param[in] text the text to set
     */
    void text(std::u16string_view text);

    /**
     * Clears the button's text
     */
    void clear();

    /**
     * Indicates if the button is currently pressed
     */
    bool pressed() const noexcept
    {
        return m_pressed;
    }

    /**
     * Adds a click listener to the button.
     *
     * \param[in] listener the click listener to add.
     *
     * Click listeners are called when the button has been clicked.
     */
    void add_click_listener(const ClickListener& listener);

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
    void label_offset(long dx, long dy) noexcept;

    Style m_style;

    std::vector<ClickListener> m_click_listeners;

    bool m_pressed{false};
    bool m_mouse_over{false};

    std::shared_ptr<Frame> m_frame;
    std::shared_ptr<Label> m_label;
};

} // namespace khepri::ui
