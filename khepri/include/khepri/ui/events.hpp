#pragma once

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

/// Identifies a key on the keyboard
enum class KeyCode : std::uint16_t
{
    space         = 32,
    apostrophe    = 39,
    comma         = 44,
    minus         = 45,
    period        = 46,
    slash         = 47,
    row_0         = 48,
    row_1         = 49,
    row_2         = 50,
    row_3         = 51,
    row_4         = 52,
    row_5         = 53,
    row_6         = 54,
    row_7         = 55,
    row_8         = 56,
    row_9         = 57,
    semicolon     = 59,
    equal         = 61,
    a             = 65,
    b             = 66,
    c             = 67,
    d             = 68,
    e             = 69,
    f             = 70,
    g             = 71,
    h             = 72,
    i             = 73,
    j             = 74,
    k             = 75,
    l             = 76,
    m             = 77,
    n             = 78,
    o             = 79,
    p             = 80,
    q             = 81,
    r             = 82,
    s             = 83,
    t             = 84,
    u             = 85,
    v             = 86,
    w             = 87,
    x             = 88,
    y             = 89,
    z             = 90,
    left_bracket  = 91,
    backslash     = 92,
    right_bracket = 93,
    grave_accent  = 96,
    escape        = 256,
    enter         = 257,
    tab           = 258,
    backspace     = 259,
    insert        = 260,
    del           = 261,
    right         = 262,
    left          = 263,
    down          = 264,
    up            = 265,
    page_up       = 266,
    page_down     = 267,
    home          = 268,
    end           = 269,
    caps_lock     = 280,
    scroll_lock   = 281,
    num_lock      = 282,
    print_screen  = 283,
    pause         = 284,

    // Function keys
    f1  = 290,
    f2  = 291,
    f3  = 292,
    f4  = 293,
    f5  = 294,
    f6  = 295,
    f7  = 296,
    f8  = 297,
    f9  = 298,
    f10 = 299,
    f11 = 300,
    f12 = 301,
    f13 = 302,
    f14 = 303,
    f15 = 304,
    f16 = 305,
    f17 = 306,
    f18 = 307,
    f19 = 308,
    f20 = 309,
    f21 = 310,
    f22 = 311,
    f23 = 312,
    f24 = 313,
    f25 = 314,

    // Keypad keys
    kp_0        = 320,
    kp_1        = 321,
    kp_2        = 322,
    kp_3        = 323,
    kp_4        = 324,
    kp_5        = 325,
    kp_6        = 326,
    kp_7        = 327,
    kp_8        = 328,
    kp_9        = 329,
    kp_decimal  = 330,
    kp_divide   = 331,
    kp_multiply = 332,
    kp_subtract = 333,
    kp_add      = 334,
    kp_enter    = 335,
    kp_equal    = 336,

    // Special keys/Modifiers
    left_shift    = 340,
    left_control  = 341,
    left_alt      = 342,
    left_super    = 343,
    right_shift   = 344,
    right_control = 345,
    right_alt     = 346,
    right_super   = 347,
    menu          = 348,
};

/// Identifies a key action.
enum class KeyAction : std::uint8_t
{
    // The key was pressed
    pressed,
    // The key was released
    released,
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

/// Represents a key event.
struct KeyEvent
{
    // The key code of the key that was pressed or released
    KeyCode code;

    // The action that was performed on the key (pressed or released).
    KeyAction action;

    // The key modifiers that were pressed when the key was pressed or released.
    KeyModifiers modifiers;
};

using InputEvent = std::variant<MouseMoveEvent, MouseButtonEvent, MouseScrollEvent, KeyEvent>;

class InputEventHandler
{
public:
    virtual ~InputEventHandler() = default;

    virtual bool handle_event(const InputEvent& event) = 0;
};

/// Event: mouse cursor leaves a widget
struct MouseLeaveEvent
{
    /// The cursor position
    MousePosition cursor_position;
};

/// Event: mouse cursor enters a widget
struct MouseEnterEvent
{
    /// The cursor position
    MousePosition cursor_position;
};

/// Event: mouse button is pressed
struct MousePressEvent
{
    /// The cursor position
    MousePosition cursor_position;

    /// The mouse button that was pressed
    MouseButton button;

    // The key modifiers that were pressed when the mouse button was pressed or released.
    KeyModifiers modifiers;
};

/// Event mouse button is released
struct MouseReleaseEvent
{
    /// The cursor position
    MousePosition cursor_position;

    /// The mouse button that was released
    MouseButton button;

    // The key modifiers that were pressed when the mouse button was pressed or released.
    KeyModifiers modifiers;
};

/// Represents a key-press event.
struct KeyPressEvent
{
    // The key code of the key that was pressed.
    KeyCode code;

    // The key modifiers that were pressed when the key was pressed.
    KeyModifiers modifiers;
};

/// Represents a key-release event.
struct KeyReleaseEvent
{
    // The key code of the key that was released
    KeyCode code;

    // The key modifiers that were pressed when the key was released.
    KeyModifiers modifiers;
};

/// Represents an event where the widget gains keyboard focus.
struct GainFocusEvent
{};

/// Represents an event where the widget losing keyboard focus.
struct LoseFocusEvent
{};

/// A UI event
using Event = std::variant<MouseMoveEvent, MouseEnterEvent, MouseLeaveEvent, MousePressEvent,
                           MouseReleaseEvent, MouseScrollEvent, KeyPressEvent, KeyReleaseEvent,
                           GainFocusEvent, LoseFocusEvent>;

class EventHandler
{
public:
    virtual ~EventHandler() = default;

    virtual bool handle_event(const Event& event) = 0;
};

} // namespace khepri::ui
