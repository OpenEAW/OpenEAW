#include <khepri/adapters/window_input.hpp>

namespace khepri {

WindowInputEventGenerator::WindowInputEventGenerator(application::Window& window)
{
    window.add_cursor_position_listener([this](const Pointi& pos) {
        dispatch_event(ui::MouseMoveEvent{ui::MousePosition{pos.x, pos.y}});
    });

    window.add_mouse_button_listener([this](const Pointi&                          pos,
                                            application::Window::MouseButton       button,
                                            application::Window::MouseButtonAction action,
                                            application::Window::KeyModifiers      modifiers) {
        dispatch_event(ui::MouseButtonEvent{
            ui::MousePosition{pos.x, pos.y}, static_cast<ui::MouseButton>(button),
            static_cast<ui::MouseButtonAction>(action), static_cast<ui::KeyModifiers>(modifiers)});
    });

    window.add_mouse_scroll_listener([this](const Pointi& pos, const Vector2& scroll_offset) {
        dispatch_event(ui::MouseScrollEvent{ui::MousePosition{pos.x, pos.y}, scroll_offset.x,
                                            scroll_offset.y});
    });

    window.add_key_listener([this](application::Window::KeyCode      key,
                                   application::Window::KeyAction    action,
                                   application::Window::KeyModifiers modifiers) {
        dispatch_event(ui::KeyEvent{static_cast<ui::KeyCode>(key),
                                    static_cast<ui::KeyAction>(action),
                                    static_cast<ui::KeyModifiers>(modifiers)});
    });
}

void WindowInputEventGenerator::add_event_handler(ui::InputEventHandler* handler)
{
    assert(handler != nullptr && "Cannot add a null handler to WindowInputEventGenerator");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_handlers.push_back(handler);
}

void WindowInputEventGenerator::remove_event_handler(ui::InputEventHandler* handler)
{
    assert(handler != nullptr && "Cannot remove a null handler from WindowInputEventGenerator");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_handlers.erase(std::remove(m_handlers.begin(), m_handlers.end(), handler), m_handlers.end());
}

void WindowInputEventGenerator::dispatch_event(const ui::InputEvent& event) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto* handler : m_handlers) {
        assert(handler != nullptr);
        if (handler->handle_event(event)) {
            // Stop further propagation if the event was handled
            break;
        }
    }
}

} // namespace khepri