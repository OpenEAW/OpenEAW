#pragma once

#include "matrix.hpp"
#include "quaternion.hpp"
#include "vector3.hpp"

#include <cassert>
#include <cmath>

namespace khepri {

//! A 3D plane
//! The normal is pointing to the "above" side of the plane
class Plane final
{
public:
    /**
     * \brief Constructs a new plane from a point and normal vector.
     *
     * Defines a plane by a point on the plane, and a vector orthogonal to the plane
     *
     * \note \a normal must be normalized
     */
    Plane(const Vector3& position, const Vector3& normal) noexcept
        : m_position(position), m_normal(normal)
    {
        assert(normal.normalized());
    }

    /// Returns the point on the plane
    [[nodiscard]] const Vector3& position() const noexcept
    {
        return m_position;
    }

    /// Returns the normal vector of plane (guaranteed normalized)
    [[nodiscard]] const Vector3& normal() const noexcept
    {
        return m_normal;
    }

    /// Returns a copy of this plane, transformed by \a transform
    [[nodiscard]] Plane transform(const Matrix& transform) const noexcept
    {
        return {transform.transform_coord(m_position), normalize(m_normal * transform)};
    }

    //! Returns the orthogonal distance between the point and the plane,
    //! where positive is "above" the plane (same direction as normal vector),
    //! and negative is "below".
    [[nodiscard]] double signed_distance(const Vector3& point) const noexcept
    {
        return dot(point - m_position, m_normal);
    }

    //! Returns the absolute, orthogonal distance between the point and the plane.
    [[nodiscard]] double distance(const Vector3& point) const noexcept
    {
        return std::abs(signed_distance(point));
    }

private:
    Vector3 m_position;
    Vector3 m_normal;
};

} // namespace khepri