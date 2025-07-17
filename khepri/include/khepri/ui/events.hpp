#pragma once

#include <khepri/application/window.hpp>

#include <mutex>
#include <optional>
#include <variant>
#include <vector>

namespace khepri::ui {

// The position of the cursor relative to the window's render area, in pixels.
struct MousePosition
{
    long x, y;
};

/// Identifies a mouse button.
enum class MouseButton
{
    /// The left mouse button
    left,
    /// The right mouse button
    right,
    /// The middle mouse button
    middle,
};

/// Identifies a mouse button action.
enum class MouseButtonAction
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

/// Represents a mouse move event.
struct MouseMoveEvent
{
    // The new position of the cursor relative to the window's render area, in pixels.
    MousePosition position;
};

/// Represents a mouse scroll event.
struct MouseScrollEvent
{
    // The current position of the cursor relative to the window's render area, in pixels.
    MousePosition position;

    // The scroll offset, in mouse wheel 'clicks'.
    // Positive X indicates scrolling right, negative X indicates scrolling left.
    // Positive Y indicates scrolling up (away from user), negative Y indicates scrolling down
    // (towards to user).
    double scroll_x, scroll_y;
};

/// Represents a mouse button event.
struct MouseButtonEvent
{
    // The position of the cursor relative to the window's render area, in pixels.
    MousePosition position;

    // The mouse button that was pressed or released.
    MouseButton button;

    // The action that was performed on the mouse button (pressed or released).
    MouseButtonAction action;

    // The key modifiers that were pressed when the mouse button was pressed or released.
    KeyModifiers modifiers;
};

using InputEvent = std::variant<MouseMoveEvent, MouseButtonEvent, MouseScrollEvent>;

class InputEventHandler
{
public:
    virtual ~InputEventHandler() = default;

    virtual bool HandleEvent(const InputEvent& event) = 0;
};

} // namespace khepri::ui