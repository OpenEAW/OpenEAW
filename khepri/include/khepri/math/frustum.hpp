#pragma once

#include "plane.hpp"
#include "sphere.hpp"
#include "vector3.hpp"

#include <cassert>

namespace khepri {

/**
 * \brief 3D frustum
 *
 * A frustum is a bounded volume defined as a trapezoid with the top cut off; it consists of two
 * parallel planes (near and far), and four side planes (left, right, top and bottom).
 *
 * It is commonly used to define a perspective camera where the near and far planes are the
 * z-near and z-far planes.
 */
class Frustum final
{
public:
    /**
     * Constructs a frustum from its six planes
     * \param[in] left the left plane
     * \param[in] right the right plane
     * \param[in] top the top plane
     * \param[in] bottom the bottom plane
     * \param[in] near_ the near plane
     * \param[in] far_ the far plane
     *
     * \note it is undefined behavior if the six planes do not have their normals vectors pointing
     *       into the frustum, or if the near and far planes are not parallel.
     */
    Frustum(const Plane& left, const Plane& right, const Plane& top, const Plane& bottom,
            const Plane& near_, const Plane& far_) noexcept
        : m_left(left), m_right(right), m_bottom(bottom), m_top(top), m_near(near_), m_far(far_)
    {
        // Sanity check: each of the pairs should point towards each other
        assert(dot(m_right.position() - m_left.position(), m_left.normal()) > 0);
        assert(dot(m_left.position() - m_right.position(), m_right.normal()) > 0);
        assert(dot(m_bottom.position() - m_top.position(), m_top.normal()) > 0);
        assert(dot(m_top.position() - m_bottom.position(), m_bottom.normal()) > 0);
        assert(dot(m_near.position() - m_far.position(), m_far.normal()) > 0);
        assert(dot(m_far.position() - m_near.position(), m_near.normal()) > 0);

        // Sanity check: the near and far planes must be parallel (dot product is -1)
        constexpr auto max_parallel_dot_product = 0.0001;
        assert(std::abs(dot(m_far.normal(), m_near.normal()) + 1) < max_parallel_dot_product);
    }

    /**
     * \brief Transform the frustum by transforming the coordinate space
     * \param[in] transform the matrix by which to transform the frustum
     * \returns a new frustum, transformed by \a transform.
     */
    [[nodiscard]] Frustum transform(const Matrix& transform) const noexcept
    {
        return Frustum(m_left.transform(transform), m_right.transform(transform),
                       m_top.transform(transform), m_bottom.transform(transform),
                       m_near.transform(transform), m_far.transform(transform));
    }

    /**
     * Checks if any part of \a sphere intersects with this frustum
     */
    [[nodiscard]] bool intersects(const Sphere& sphere) const noexcept
    {
        // Returns true if any part of the sphere is above the plane
        auto above = [&sphere](const Plane& plane) {
            return plane.signed_distance(sphere.center()) > -sphere.radius();
        };

        return above(m_left) && above(m_right) && above(m_top) && above(m_bottom) &&
               above(m_near) && above(m_far);
    }

    /**
     * Checks if the point represented by \a v is inside this frustum
     */
    [[nodiscard]] bool inside(const Vector3& v) const noexcept
    {
        // Returns true if the vector is above the plane
        const auto above = [&](const Plane& plane) { return plane.signed_distance(v) >= 0.0F; };

        return above(m_left) && above(m_right) && above(m_top) && above(m_bottom) &&
               above(m_near) && above(m_far);
    }

    /**
     * Checks if \a sphere is entirely contained in this frustum
     */
    [[nodiscard]] bool inside(const Sphere& sphere) const noexcept
    {
        // Returns true if the whole sphere is above the plane
        auto above = [&sphere](const Plane& plane) {
            return plane.signed_distance(sphere.center()) >= sphere.radius();
        };

        return above(m_left) && above(m_right) && above(m_top) && above(m_bottom) &&
               above(m_near) && above(m_far);
    }

private:
    Plane m_left, m_right;
    Plane m_bottom, m_top;
    Plane m_near, m_far;
};

} // namespace khepri