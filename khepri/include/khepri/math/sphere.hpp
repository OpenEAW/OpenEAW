#pragma once

#include "vector3.hpp"

#include <cassert>

namespace khepri {

/**
 * \brief Sphere
 */
class Sphere final
{
public:
    /// Constructs a sphere from a center and radius
    Sphere(const Vector3& center, float radius) noexcept : m_center(center), m_radius(radius)
    {
        assert(radius >= 0.0F);
    }

    /// Returns the center of the sphere
    [[nodiscard]] const Vector3& center() const noexcept
    {
        return m_center;
    }

    /// Returns the radius of the sphere
    [[nodiscard]] float radius() const noexcept
    {
        return m_radius;
    }

    /// Returns squared radius of the sphere
    [[nodiscard]] float radius_sq() const noexcept
    {
        return m_radius * m_radius;
    }

    /// Checks if the point represented by \a v lies inside the sphere
    [[nodiscard]] bool inside(const Vector3& v) const noexcept
    {
        return (v - m_center).length_sq() < radius_sq();
    }

    /// Translates (moves) the sphere by \a v
    [[nodiscard]] Sphere translate(const Vector3& v) const noexcept
    {
        return Sphere(m_center + v, m_radius);
    }

    /// Scales the sphere by \a scale
    [[nodiscard]] Sphere scale(float scale) const noexcept
    {
        return Sphere(m_center, m_radius * scale);
    }

private:
    Vector3 m_center;
    float   m_radius;
};

} // namespace khepri