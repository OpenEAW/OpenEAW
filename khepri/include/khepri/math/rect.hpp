#pragma once

#include "point.hpp"

#include <algorithm>
#include <optional>

namespace khepri {

/**
 * \brief Represents a discrete, two-dimensional rectangle
 *
 * By convention, the rectangle's width and height are exclusive: the positions described by x
 * + width or y + height are considered to be outside of the rectangle.
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

inline constexpr bool operator==(const Rect& r1, const Rect& r2) noexcept
{
    return r1.x == r2.x && r1.y == r2.y && r1.width == r2.width && r1.height == r2.height;
}

inline constexpr bool operator!=(const Rect& r1, const Rect& r2) noexcept
{
    return !(r1 == r2);
}

/**
 * Determines if a point is inside a rectangle.
 *
 * \param[in] p the point
 * \param[in] r the rectangle
 *
 * The rectangle's width and height are exclusive: the positions described by x + width or y +
 * height are considered to be outside of the rectangle.
 */
constexpr bool inside(const Pointi& p, const Rect& r)
{
    return static_cast<long>(p.x) >= r.x && static_cast<long>(p.y) >= r.y &&
           static_cast<unsigned long>(static_cast<long>(p.x) - r.x) < r.width &&
           static_cast<unsigned long>(static_cast<long>(p.y) - r.y) < r.height;
}

/**
 * Offsets a rectangle.
 *
 * \param[in] r the rectangle
 * \param[in] ofs the offset
 * \returns a copy of \a r with its x and y members offset by \a ofs. Its width and height are not
 * modified.
 */
inline constexpr Rect offset(const Rect& r, const Pointi& ofs) noexcept
{
    return {r.x + ofs.x, r.y + ofs.y, r.width, r.height};
}

/**
 * Intersects two rectangles
 *
 * \param[in] r1 the first rectangle
 * \param[in] r2 the second rectangle
 *
 * \returns the intersection of \a r1 and \a r2 or \a std::nullopt if there is no intersection.
 *
 * The intersection of rectangles \a r1 and \a r2 is the largest rectangle whose points lie inside
 * both \a r1 and \a r2. Its width and height are guaranteed to be greater than zero.
 */
inline constexpr std::optional<Rect> intersect(const Rect& r1, const Rect& r2) noexcept
{
    const auto r1_right  = static_cast<long>(r1.x + r1.width);
    const auto r2_right  = static_cast<long>(r2.x + r2.width);
    const auto r1_bottom = static_cast<long>(r1.y + r1.height);
    const auto r2_bottom = static_cast<long>(r2.y + r2.height);

    const bool overlap_x = (r2.x < r1_right) && (r1.x < r2_right);
    const bool overlap_y = (r2.y < r1_bottom) && (r1.y < r2_bottom);

    if (!overlap_x || !overlap_y) {
        return {};
    }

    Rect r{};
    r.x      = std::max(r1.x, r2.x);
    r.y      = std::max(r1.y, r2.y);
    r.width  = static_cast<unsigned long>(std::min(r1_right, r2_right) - r.x);
    r.height = static_cast<unsigned long>(std::min(r1_bottom, r2_bottom) - r.y);

    if (r.width == 0 || r.height == 0) {
        return {};
    }

    return r;
}

/**
 * Combines two rectangles into their union
 *
 * \param[in] r1 the first rectangle
 * \param[in] r2 the second rectangle
 *
 * \returns the union of \a r1 and \a r2.
 *
 * The union of rectangles \a r1 and \a r2 is the smallest rectangle such that both \a r1 and \r2
 * are both inside that rectangle.
 */
inline constexpr Rect combine(const Rect& r1, const Rect& r2) noexcept
{
    const auto r1_right  = static_cast<long>(r1.x + r1.width);
    const auto r2_right  = static_cast<long>(r2.x + r2.width);
    const auto r1_bottom = static_cast<long>(r1.y + r1.height);
    const auto r2_bottom = static_cast<long>(r2.y + r2.height);

    Rect r{};
    r.x      = std::min(r1.x, r2.x);
    r.y      = std::min(r1.y, r2.y);
    r.width  = static_cast<unsigned long>(std::max(r1_right, r2_right) - r.x);
    r.height = static_cast<unsigned long>(std::max(r1_bottom, r2_bottom) - r.y);
    return r;
}

} // namespace khepri