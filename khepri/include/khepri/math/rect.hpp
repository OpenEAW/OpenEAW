#pragma once

#include "point.hpp"

namespace khepri {

/**
 * \brief Represents a discrete, two-dimensional rectangle
 */
struct Rect
{
    /// The x coordinate of the top-left point of the rectangle
    long x;

    /// The y coordinate of the top-left point of the rectangle
    long y;

    /// The width of the rectangle
    unsigned long width;

    /// The height of the rectangle
    unsigned long height;
};

/**
 * Determines if a point is inside a rectangle.
 *
 * \param[in] p the point
 * \param[in] r the rectangle
 *
 * The rectangle's width and height are exclusive: the positions described by x + width or y +
 * height are considered to be outside of the rectangle.
 */
constexpr bool inside(const Point& p, const Rect& r)
{
    return static_cast<long>(p.x) >= r.x && static_cast<long>(p.y) >= r.y &&
           static_cast<unsigned long>(static_cast<long>(p.x) - r.x) < r.width &&
           static_cast<unsigned long>(static_cast<long>(p.y) - r.y) < r.height;
}

} // namespace khepri