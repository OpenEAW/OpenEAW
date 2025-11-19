#pragma once

#include <khepri/math/point.hpp>
#include <khepri/math/size.hpp>

namespace khepri::ui {

/**
 * Anchor points for a widget.
 *
 * These define how a widget moves and/or resizes in relation to its parent.
 */
struct Anchor
{
    /// Fraction of the parent's axis to anchor to (0.0 - 1.0)
    float parent_frac;

    /// Offset relative to anchor
    long offset;
};

/**
 * Layout information for a widget
 */
struct Layout
{
    /// Anchor information for the widget's left side
    Anchor left;
    /// Anchor information for the widget's top side
    Anchor top;
    /// Anchor information for the widget's right side
    Anchor right;
    /// Anchor information for the widget's bottom side
    Anchor bottom;

    /// Convenience function for getting a fill layout: the widget stretches to fill its parent
    static Layout fill() noexcept
    {
        return {{0.0f, 0}, {0.0f, 0}, {1.0f, 0}, {1.0f, 0}};
    }

    /**
     * Convenience function for getting a top-left relative layout: the widget doesn't resize
     * and stays relative to the top-left corner of its parent.
     *
     * \param[in] offset offset from top-left corner of parent
     * \param[in] size size of widget
     */
    static Layout top_left(const Pointi& offset, const Size& size) noexcept
    {
        return {{0.0f, offset.x},
                {0.0f, offset.y},
                {0.0f, offset.x + static_cast<long>(size.width)},
                {0.0f, offset.y + static_cast<long>(size.height)}};
    }

    /**
     * Convenience function for getting a top-right relative layout: the widget doesn't resize
     * and stays relative to the top-right corner of its parent.
     *
     * \param[in] offset offset from top-right corner of parent
     * \param[in] size size of widget
     *
     * \note the offset is not inverted, so a positive X offset puts the layout outside of the
     * parent. A negative X offset is recommended for most use cases.
     */
    static Layout top_right(const Pointi& offset, const Size& size) noexcept
    {
        return {{1.0f, offset.x},
                {0.0f, offset.y},
                {1.0f, offset.x + static_cast<long>(size.width)},
                {0.0f, offset.y + static_cast<long>(size.height)}};
    }

    /**
     * Convenience function for getting a centered layout: the widget doesn't resize
     * and stays relative to the center of its parent
     *
     * \param[in] size size of widget
     */
    static Layout center(const Size& size) noexcept
    {
        const long x = -static_cast<long>(size.width) / 2;
        const long y = -static_cast<long>(size.height) / 2;
        return {{0.5f, x},
                {0.5f, y},
                {0.5f, x + static_cast<long>(size.width)},
                {0.5f, y + static_cast<long>(size.height)}};
    }
};

} // namespace khepri::ui
