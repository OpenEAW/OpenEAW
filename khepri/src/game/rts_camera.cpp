#include <khepri/game/rts_camera.hpp>

#include <cassert>
#include <iostream>

namespace khepri::game {
namespace {
template <typename T>
T apply_constraint(const khepri::Constraint<T>& constraint, const T& old_value, const T& new_value)
{
    if (constraint) {
        return constraint(old_value, new_value);
    }
    return new_value;
}

double update_free_property(const RtsCameraController::FreeProperty& property, double old_value,
                            double diff)
{
    return khepri::clamp(old_value + diff * property.sensitivity, property.constraint.min,
                         property.constraint.max);
}

} // namespace

RtsCameraController::RtsCameraController(renderer::Camera& camera, const Point& target)
    : m_camera(camera)
    , m_target(target)
    , m_distance_property(
          {std::make_unique<LinearInterpolator>(std::vector<Point>{{0, 10.0}, {1, 1000.0}}), 0.1})
    , m_fov_property(
          {std::make_unique<LinearInterpolator>(std::vector<Point>{{0, PI / 4}, {1, PI / 4}}), 0.1})
    , m_yaw_property(FreeProperty{Range{0.0, PI / 2}, 1.0})
    , m_pitch_property(FreeProperty{Range{0.0, PI / 2}, 1.0})
{
    update_camera();
}

void RtsCameraController::update_camera() const
{
    const auto& dir = direction();
    m_camera.position(Vector3{m_target.x, m_target.y, 0} - dir * distance());
    // Note: the camera target can be any position in front of the camera
    m_camera.target(m_camera.position() + dir);
    m_camera.up(up());
    m_camera.fov(m_fov.value());
}

void RtsCameraController::target_constraint(const khepri::Constraint<Point>& constraint)
{
    m_target_constraint = constraint;
    m_target            = m_target_constraint(m_target, m_target);
    update_camera();
}

void RtsCameraController::rotate(double yaw_angle_diff, double pitch_angle_diff)
{
    m_yaw.target(update_free_property(m_yaw_property, m_yaw.target(), yaw_angle_diff));
    if (const auto* free_pitch = std::get_if<FreeProperty>(&m_pitch_property)) {
        m_pitch.target(update_free_property(*free_pitch, m_pitch.target(), pitch_angle_diff));
    }
}

void RtsCameraController::translate(const Vector2& camera_offset)
{
    const auto camera_offset_length = camera_offset.length();
    if (std::abs(camera_offset_length) < 0.000001) {
        return;
    }
    const auto camera_offset_direction = normalize(camera_offset);
    const auto camera_direction        = direction();
    const auto world_offset =
        normalize(normalize(Vector2(right())) * camera_offset_direction.x +
                  normalize(Vector2(camera_direction)) * camera_offset_direction.y) *
        camera_offset_length;
    m_target = apply_constraint(m_target_constraint, m_target, m_target + world_offset);
    update_camera();
}

void RtsCameraController::target(const Point& target)
{
    m_target = apply_constraint(m_target_constraint, m_target, target);
    update_camera();
}

void RtsCameraController::zoom(double amount)
{
    m_zoom_level = clamp(m_zoom_level - amount * m_zoom_sensitivity, 0.0, 1.0);

    // Update zoom-dependent properties
    m_distance.target(m_distance_property.interpolator->interpolate(m_zoom_level));
    m_fov.target(m_fov_property.interpolator->interpolate(m_zoom_level));
    if (const auto* zoom_pitch = std::get_if<ZoomProperty>(&m_pitch_property)) {
        m_pitch.target(zoom_pitch->interpolator->interpolate(m_zoom_level));
    }
}

void RtsCameraController::zoom_sensitivity(double sensitivity) noexcept
{
    m_zoom_sensitivity = sensitivity;
}

double RtsCameraController::distance() const noexcept
{
    return m_distance.value();
}

void RtsCameraController::distance_property(ZoomProperty property)
{
    m_distance_property = std::move(property);
    m_distance.target(m_distance_property.interpolator->interpolate(m_zoom_level));
    m_distance.smooth_time(m_distance_property.smooth_time);
    update_camera();
}

void RtsCameraController::yaw_property(FreeProperty constraint)
{
    m_yaw_property = std::move(constraint);
    m_yaw.target(update_free_property(m_yaw_property, m_yaw.target(), 0));
    m_yaw.smooth_time(m_yaw_property.smooth_time);
    update_camera();
}

void RtsCameraController::pitch_property(PitchProperty property)
{
    m_pitch_property = std::move(property);
    if (const auto* free_pitch = std::get_if<FreeProperty>(&m_pitch_property)) {
        m_pitch.target(update_free_property(*free_pitch, m_pitch.value(), 0));
        m_pitch.smooth_time(free_pitch->smooth_time);
    } else if (const auto* zoom_pitch = std::get_if<ZoomProperty>(&m_pitch_property)) {
        m_pitch.target(zoom_pitch->interpolator->interpolate(m_zoom_level));
        m_pitch.smooth_time(zoom_pitch->smooth_time);
    }
    update_camera();
}

void RtsCameraController::fov_property(ZoomProperty property)
{
    m_fov_property = std::move(property);
    m_fov.target(m_fov_property.interpolator->interpolate(m_zoom_level));
    m_yaw.smooth_time(m_fov_property.smooth_time);
    update_camera();
}

Vector3 RtsCameraController::direction() const noexcept
{
    // Pitch should cause the camera to tilt _down_, so invert the pitch.
    return Vector3::from_angles(-m_pitch.value(), m_yaw.value());
}

Vector3 RtsCameraController::up() const noexcept
{
    const auto r = std::sin(m_pitch.value());
    return {r * std::cos(m_yaw.value()), r * std::sin(m_yaw.value()), std::cos(m_pitch.value())};
}

Vector3 RtsCameraController::right() const noexcept
{
    return cross(direction(), up());
}

void RtsCameraController::update(double dt)
{
    m_distance.update(dt);
    m_fov.update(dt);
    m_yaw.update(dt);
    m_pitch.update(dt);

    update_camera();
};

void RtsCameraController::update_immediate()
{
    m_distance.update_immediate();
    m_fov.update_immediate();
    m_yaw.update_immediate();
    m_pitch.update_immediate();
    update_camera();
}

} // namespace khepri::game