#include <openglyph/ui/input.hpp>

namespace openglyph::ui {
namespace {

// The distance (as a fraction of the window) that the mouse must move before a drag is initiated
// (in classic scroll mode).
constexpr auto MIN_CLASSIC_SCROLL_MODE_DRAG_DISTANCE = 0.08;

// Multiplication factor for the camera height for the scroll distance, so that the camera scrolls
// faster when it is further away from the ground.
constexpr auto SCROLL_DISTANCE_MULTIPLIER = 0.01;

khepri::Vector2 to_screen_space(const khepri::ui::MousePosition& pos,
                                const khepri::Size&              render_size)
{
    return {static_cast<double>(pos.x) / render_size.width,
            static_cast<double>(pos.y) / render_size.height};
}
} // namespace

TacticalModeInputHandler::TacticalModeInputHandler(
    khepri::game::RtsCameraController& camera_controller, khepri::application::Window& window)
    : m_camera_controller(camera_controller), m_window(window)
{
}

bool TacticalModeInputHandler::HandleEvent(const khepri::ui::InputEvent& event)
{
    if (auto* mouse_move = std::get_if<khepri::ui::MouseMoveEvent>(&event)) {
        const auto cursor_screen_pos =
            to_screen_space(mouse_move->position, m_window.render_size());
        if (m_drag_state) {
            if (!m_drag_state->is_modifying_camera &&
                m_drag_state->button == khepri::ui::MouseButton::right) {
                // Classic scroll mode: check if we can start modifying the camera
                const auto drag_distance =
                    std::max(std::abs(cursor_screen_pos.x - m_drag_state->start.x),
                             std::abs(cursor_screen_pos.y - m_drag_state->start.y));
                if (drag_distance >= MIN_CLASSIC_SCROLL_MODE_DRAG_DISTANCE) {
                    m_drag_state->is_modifying_camera = true;
                }
            }

            if (m_drag_state->is_modifying_camera) {
                const auto drag_diff = cursor_screen_pos - m_prev_cursor_screen_pos;
                if (m_drag_state->modifiers == khepri::ui::KeyModifiers::ctrl) {
                    // Ctrl + middle mouse button: rotate camera
                    constexpr auto rotate_multiplier = 100;
                    RotateCamera(rotate_multiplier * drag_diff.x, rotate_multiplier * drag_diff.y);
                } else {
                    // Middle mouse button: translate camera
                    constexpr auto move_multiplier = 400;
                    TranslateCamera(move_multiplier * drag_diff.x, move_multiplier * drag_diff.y);
                }
            }
        }
        m_prev_cursor_screen_pos = cursor_screen_pos;
        return true;

    } else if (auto* mouse_button = std::get_if<khepri::ui::MouseButtonEvent>(&event)) {
        if (mouse_button->action == khepri::ui::MouseButtonAction::pressed) {
            if (!m_drag_state) {
                m_drag_state = {mouse_button->button, mouse_button->modifiers,
                                to_screen_space(mouse_button->position, m_window.render_size()),
                                mouse_button->button == khepri::ui::MouseButton::middle};
                m_window.set_infinite_cursor(true);
            }
        } else if (m_drag_state && mouse_button->button == m_drag_state->button) {
            m_drag_state = std::nullopt;
            m_window.set_infinite_cursor(false);
        }
    } else if (auto* mouse_scroll = std::get_if<khepri::ui::MouseScrollEvent>(&event)) {
        ZoomCamera(mouse_scroll->scroll_y);
        return true;
    }
    // Event not handled
    return false;
}

void TacticalModeInputHandler::RotateCamera(double x, double y)
{
    // Flip X and Y, because rotating left (negative X) should rotate the camera right. And rotating
    // down (negative Y) should rotate the camera up.
    m_camera_controller.rotate(khepri::to_radians(-x), khepri::to_radians(-y));
}

void TacticalModeInputHandler::TranslateCamera(double x, double y)
{
    // Scroll faster the higher the camera is
    const auto dist_mult = SCROLL_DISTANCE_MULTIPLIER * m_camera_controller.distance();

    x *= dist_mult;
    y *= dist_mult;

    const auto right   = normalize(khepri::Vector2(m_camera_controller.right()));
    const auto forward = normalize(khepri::Vector2(m_camera_controller.direction()));
    // Flip Y, because on-screen "up" (negative Y) should move forward (positive Y).
    m_camera_controller.target(m_camera_controller.target() + right * x + forward * -y);
}

void TacticalModeInputHandler::ZoomCamera(double amount)
{
    m_camera_controller.zoom(amount);
}

} // namespace openglyph::ui