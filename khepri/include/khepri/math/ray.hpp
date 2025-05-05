#pragma once

#include "matrix.hpp"
#include "quaternion.hpp"
#include "sphere.hpp"
#include "vector3.hpp"

#include <cassert>
#include <cmath>

namespace khepri {

/**
 * \brief A 3D ray
 *
 * A ray is defined by a starting point and a direction. It is infinite in one direction only.
 */
class Ray final
{
public:
    /**
     * \brief Constructs the ray from a starting point and direction.
     *
     * \note \a direction must be normalized
     */
    Ray(const Vector3& start, const Vector3& direction) noexcept
        : m_start(start), m_direction(direction)
    {
        assert(direction.normalized());
    }

    /// Returns the starting point of the ray
    [[nodiscard]] const Vector3& start() const noexcept
    {
        return m_start;
    }

    /// Returns the direction of the ray
    [[nodiscard]] const Vector3& direction() const noexcept
    {
        return m_direction;
    }

    /// Returns a copy of this ray, transformed by \a transform
    [[nodiscard]] Ray transform(const Matrix& transform) const noexcept
    {
        return Ray(transform.transform_coord(m_start), normalize(m_direction * transform));
    }

    /**
     * \brief Find intersection distance with \a sphere.
     *
     * Returns the distance along the ray of the first intersection with the sphere, if any.
     * Returns a negative number if there is no intersection or if the starting point is inside the
     * sphere.
     */
    [[nodiscard]] float intersect_distance(const Sphere& sphere) const noexcept
    {
        // vector from ray origin (O) to sphere center (C)
        Vector3 OC = sphere.center() - m_start;

        // Distance along ray of orthogonal projection (P) of sphere center (C)
        float dist_P = dot(OC, m_direction);

        // Sphere center must be in front of ray start, or no intersection
        if (dist_P >= 0.0F) {
            // Squared distance from sphere center (C) to ray (P)
            float dist_to_ray_sq = OC.length_sq() - dist_P * dist_P;

            // P must be inside of sphere, or no intersection
            float radius_sq = sphere.radius_sq();
            if (dist_to_ray_sq <= radius_sq) {
                // Distance along ray of intersection with sphere
                float d = dist_P - sqrt(radius_sq - dist_to_ray_sq);

                // Intersection must be in front of ray, or no intersection
                if (d >= 0.0) {
                    return d;
                }
            }
        }

        // No intersection
        return -1.0;
    }

private:
    Vector3 m_start;
    Vector3 m_direction;
};

} // namespace khepri
