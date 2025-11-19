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

            /// The button's font
            std::shared_ptr<khepri::font::Font> font;

            /// The button's text alignment
            TextAlign text_align{TextAlign::center};
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
    explicit Button(const Layout& layout)
        : Widget(layout)
        , m_frame(std::make_shared<Frame>(Layout::fill()))
        , m_label(std::make_shared<Label>(Layout::fill()))
    {
        add(m_frame);
        add(m_label);
    }

    /**
     * Sets the button's style
     *
     * \param[in] style the button's style.
     */
    void style(const Style& style) noexcept
    {
        m_style = style;
        apply_active_style();
    }

    /**
     * Retrieves the button's text
     */
    const auto& text() const noexcept
    {
        return m_label->text();
    }

    /**
     * Sets the button's text
     *
     * \param[in] text the text to set
     */
    void text(std::u16string text)
    {
        m_label->text(std::move(text));
    }

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
    void add_click_listener(const ClickListener& listener)
    {
        m_click_listeners.push_back(listener);
    }

    /// \see Widget::enabled()
    void enabled(bool enabled) noexcept override
    {
        Widget::enabled(enabled);
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

private:
    void apply_active_style() noexcept
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

    void apply_state_style(const Style::State& state_style) noexcept
    {
        m_frame->style(state_style.frame);
        m_label->style({state_style.font, state_style.text_align});
    }

    void label_offset(long dx, long dy) noexcept
    {
        m_label->layout({{0.0f, dx}, {0.0f, dy}, {1.0f, dx}, {1.0f, dy}});
    }

    Style m_style;

    std::vector<ClickListener> m_click_listeners;

    bool m_pressed{false};
    bool m_mouse_over{false};

    std::shared_ptr<Frame> m_frame;
    std::shared_ptr<Label> m_label;
};

} // namespace khepri::ui
