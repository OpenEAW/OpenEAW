#pragma once

#include <khepri/application/window.hpp>
#include <khepri/game/rts_camera.hpp>
#include <khepri/ui/events.hpp>

#include <optional>

namespace openglyph::ui {

/**
 * \brief Translates raw input events into game-specific actions for tactical mode.
 *
 * This handler processes input events such as mouse movements and button presses and translates
 * them into game-specific actions like camera movement, unit selections, and more.
 */
class TacticalModeInputHandler final : public khepri::ui::InputEventHandler
{
public:
    explicit TacticalModeInputHandler(khepri::game::RtsCameraController& camera_controller,
                                      khepri::application::Window&       window);

    bool HandleEvent(const khepri::ui::InputEvent& event) override;

private:
    void RotateCamera(double x, double y);
    void TranslateCamera(double x, double y);
    void ZoomCamera(double amount);

    struct DragState
    {
        // The mouse button that initiated the drag.
        khepri::ui::MouseButton button;

        // The key modifiers that were pressed when the drag started.
        khepri::ui::KeyModifiers modifiers;

        // The position of the cursor when the drag started, in pixels, in window space.
        khepri::Vector2 start;

        // Whether or not the drag is currently modifying the camera.
        bool is_modifying_camera{false};
    };

    khepri::game::RtsCameraController& m_camera_controller;
    khepri::application::Window&       m_window;
    std::optional<DragState>           m_drag_state;
    khepri::Vector2                    m_prev_cursor_screen_pos{0, 0};
};

} // namespace openglyph::ui