#pragma once

#include <khepri/math/point.hpp>
#include <khepri/math/size.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/utility/enum.hpp>

#include <any>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace khepri::application {

/**
 * \brief A user-visible window
 *
 * A window is the primary means of interaction by the user with the application.
 * It can provide native window handles for the renderer and receive and handle
 * input events.
 */
class Window final
{
public:
    /// Identifies a mouse button
    enum class MouseButton : std::uint8_t
    {
        /// The left mouse button
        left,
        /// The right mouse button
        right,
        /// The middle mouse button
        middle,
    };

    /// Identifies a mouse button action
    enum class MouseButtonAction : std::uint8_t
    {
        /// The mouse button was pressed
        pressed,
        /// The mouse button was released
        released
    };

    /// This is a bitmask of modifiers that can be pressed (potentially simultaneously) while a
    /// mouse button or key is pressed.
    enum class KeyModifiers : std::uint8_t
    {
        /// No modifier keys are pressed
        none = 0,
        /// The Control key
        ctrl = 1,
        /// The Alt key
        alt = 2,
        /// The Shift key
        shift = 4,
    };

    /// Callback for "window size changed" events
    using SizeListener = std::function<void()>;

    /// Callback for "cursor position changed" events
    using CursorPositionListener = std::function<void(const khepri::Pointi& pos)>;

    /// Callback for "mouse button" events
    using MouseButtonListener = std::function<void(const khepri::Pointi& pos, MouseButton,
                                                   MouseButtonAction, KeyModifiers)>;

    /// Callback for "mouse scroll" events
    /// The scroll offset's X indicates right (positive) or left (negative) scroll, Y indicates up
    /// (positive) or down (negative) scroll.
    using MouseScrollListener =
        std::function<void(const khepri::Pointi& pos, const khepri::Vector2& scroll_offset)>;

    /**
     * Constructs the window
     * \param[in] title the title of the window, as visible in the title bar.
     */
    explicit Window(const std::string& title);
    Window(const Window&)            = delete;
    Window(Window&&)                 = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&)      = delete;
    ~Window();

    /**
     * Returns the native handle of this window.
     * The returned type depends on the target platform:
     * - Windows: a HWND is returned.
     * - Linux:   a std::tuple<void*, std::uint32_t> is returned where the first element is
     *            the X11 display pointer and the second element is the X11 window ID.
     */
    [[nodiscard]] std::any native_handle() const;

    /**
     * Returns the size of the render area.
     */
    [[nodiscard]] Size render_size() const;

    /**
     * \brief Returns true if the window should close.
     *
     * This will return true if the user, system or other event indicated that the window
     * should be closed. The application is responsible for checking this flag regularly
     * and act accordingly.
     *
     * \note If this is the only window in the application, it may want to shut down.
     */
    [[nodiscard]] bool should_close() const;

    /**
     * \brief Returns true if the window's render buffers need to be swapped with this method.
     *
     * In practice, this is true for OpenGL contexts. Otherwise, the renderer should be used to
     * present the rendered content.
     */
    [[nodiscard]] static bool use_swap_buffers();

    /**
     * Swaps the front and back buffers of the window.
     */
    void swap_buffers();

    /**
     * Adds a listener for "window size changed" events.
     * If invoked, call #render_size() to obtain the new render size.
     */
    void add_size_listener(const SizeListener& listener);

    /**
     * Adds a listener for "cursor posiion changed" events.
     * The cursor's position relative to the window's render area are passed along.
     */
    void add_cursor_position_listener(const CursorPositionListener& listener);

    /**
     * Adds a listener for "mouse button" events.
     * The cursor's position relative to the window's render area, the mouse button, and the button
     * action are passed along.
     */
    void add_mouse_button_listener(const MouseButtonListener& listener);

    /**
     * Adds a listener for "mouse scroll" events.
     * The cursor's position relative to the window's render area and the amount of horizontal and
     * vertical scroll is passed along.
     */
    void add_mouse_scroll_listener(const MouseScrollListener& listener);

    /**
     * Sets the mouse cursor position.
     * The cursor's position is relative to the window's render area.
     */
    void set_cursor_position(const Pointi& position);

    /**
     * Enables or disables "infinite cursor" mode.
     *
     * In infinite cursor mode, the cursor is locked to the window and can move indefinitely without
     * leaving the window.
     */
    void set_infinite_cursor(bool infinite);

    /**
     * \brief observer and handle new events on the process's event queue.
     *
     * Every process has a single event queue that all user input events are posted to.
     * This method handles those events. The application should ensure that this method is
     * called regularly to guarantee responsiveness.
     */
    static void poll_events();

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

} // namespace khepri::application