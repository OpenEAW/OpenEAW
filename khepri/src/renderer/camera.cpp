#include <khepri/math/math.hpp>
#include <khepri/renderer/camera.hpp>

#include <algorithm>
#include <cassert>

namespace khepri::renderer {

Camera::Matrices Camera::create_matrices(const Properties& properties) noexcept
{
    // The view direction and the "up" vector may not be colinear;
    // otherwise we have a degree of freedom in the camera transformation and
    // the matrix calculations will be messed up.
    assert(!colinear(properties.up, properties.target - properties.position));

    Matrices matrices;
    matrices.view = Matrixf::create_look_at_view(
        Vector3f{properties.position}, Vector3f{properties.target}, Vector3f{properties.up});
    matrices.projection =
        (properties.type == Type::orthographic)
            ? Matrixf::create_orthographic_projection(
                  static_cast<float>(properties.width), static_cast<float>(properties.aspect),
                  static_cast<float>(properties.znear), static_cast<float>(properties.zfar))
            : Matrixf::create_perspective_projection(
                  static_cast<float>(properties.fov), static_cast<float>(properties.aspect),
                  static_cast<float>(properties.znear), static_cast<float>(properties.zfar));
    matrices.view_proj     = matrices.view * matrices.projection;
    matrices.view_inv      = inverse(matrices.view);
    matrices.view_proj_inv = inverse(matrices.view_proj);
    return matrices;
}

Camera::Camera(const Properties& properties) : m_properties(properties) {}

void Camera::type(Type type) noexcept
{
    m_properties.type = type;
    clear_cache();
}

void Camera::position(const Vector3& position) noexcept
{
    m_properties.position = position;
    clear_cache();
}

void Camera::target(const Vector3& target) noexcept
{
    m_properties.target = target;
    clear_cache();
}

void Camera::up(const Vector3& up) noexcept
{
    m_properties.up = up;
    clear_cache();
}

void Camera::fov(double fov) noexcept
{
    m_properties.fov = fov;
    clear_cache();
}

void Camera::width(double width) noexcept
{
    m_properties.width = width;
    clear_cache();
}

void Camera::aspect(double aspect) noexcept
{
    m_properties.aspect = aspect;
    clear_cache();
}

void Camera::znear(double znear) noexcept
{
    m_properties.znear = znear;
    clear_cache();
}

void Camera::zfar(double zfar) noexcept
{
    m_properties.zfar = zfar;
    clear_cache();
}

Frustum Camera::frustum(const Vector2& p1, const Vector2& p2) const noexcept
{
    const auto& m = matrices();

    // Constructs a side plane from its coordinates on the near plane (-1 <= x,y <= 1)
    // The @orthogonal_view_dir lies in the plane, orthogonal to the view direction.
    const auto create_side_plane = [&](double x, double y, const Vector3& orthogonal_view_dir) {
        const Vector3 near_position = m.view_proj_inv.transform_coord(Vector3{x, y, 0.0});
        const Vector3 far_position  = m.view_proj_inv.transform_coord(Vector3{x, y, 1.0});
        const Vector3 inside_dir =
            normalize(cross(far_position - near_position, orthogonal_view_dir));
        return Plane(near_position, inside_dir);
    };

    // Calculate world-space directions of camera-space view, right and up.
    const Vector3 view_dir  = normalize(m_properties.target - m_properties.position);
    const Vector3 right_dir = normalize(cross(view_dir, m_properties.up));
    const Vector3 up_dir    = normalize(cross(right_dir, view_dir));

    // Get the bounds
    const auto [min_x, max_x] = std::minmax(p1.x, p2.x);
    const auto [min_y, max_y] = std::minmax(p1.y, p2.y);

    // Construct the view frustum
    const Plane left   = create_side_plane(min_x, min_y, up_dir);
    const Plane right  = create_side_plane(max_x, min_y, -up_dir);
    const Plane top    = create_side_plane(min_x, max_y, right_dir);
    const Plane bottom = create_side_plane(min_x, min_y, -right_dir);
    const Plane near(m_properties.position + m_properties.znear * view_dir, view_dir);
    const Plane far(m_properties.position + m_properties.zfar * view_dir, -view_dir);

    return {left, right, top, bottom, near, far};
}

const Camera::Matrices& Camera::matrices() const noexcept
{
    if (!m_matrices) {
        m_matrices = create_matrices(m_properties);
    }
    return *m_matrices;
}

const Frustum& Camera::frustum() const noexcept
{
    if (!m_frustum) {
        m_frustum = frustum({-1, -1}, {1, 1});
    }
    return *m_frustum;
}

std::tuple<Vector3, Vector3> Camera::unproject(const Vector2& coords) const noexcept
{
    const auto& m = matrices();
    return {
        m.view_proj_inv.transform_coord(Vector3{coords, 0.0}), // Near
        m.view_proj_inv.transform_coord(Vector3{coords, 1.0})  // far
    };
}

double Camera::lod(const Vector3& world_pos) const noexcept
{
    // The LOD is just the Z-position in the view frustum
    // (but inverted: 1 is near plane, 0 is far plane)
    const auto& m = matrices();
    const auto  v = Vector4(world_pos, 1) * m.view_proj;
    return saturate((m_properties.zfar - v.w) / (m_properties.zfar - m_properties.znear));
}

void Camera::clear_cache()
{
    m_matrices = {};
    m_frustum  = {};
}

void Camera::properties(const Properties& properties) noexcept
{
    m_properties = properties;
    clear_cache();
}

} // namespace khepri::renderer
