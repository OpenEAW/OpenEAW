#pragma once

#include "polynomial.hpp"

#include <gsl/gsl-lite.hpp>

#include <initializer_list>
#include <vector>

namespace khepri {

/**
 * \brief Base class for interpolators.
 *
 * Interpolators are constructed from a sequence of (x,y) control points and offer a way to
 * calculate the interpolated y position for a given x position.
 */
class Interpolator
{
public:
    virtual ~Interpolator() = default;

    /**
     * \brief Return interpolated y value for a given x value.
     *
     * \note x is clamped to the input range for the interpolator.
     */
    [[nodiscard]] virtual double interpolate(double x) const noexcept = 0;
};

/**
 * @brief An interpolator that "steps" immediately to the next value
 *
 * This interpolator returns the \a y value of the last control point before the input \a x value.
 * In effect, this causes the interpolator to "step" to the next control point's \a y value once \a
 * x passes that point.
 *
 * This is the computationally cheapest interpolator, but is not continuous in any way.
 */
class StepInterpolator final : public Interpolator
{
public:
    /**
     * \brief Constructs a new StepInterpolator from an \a initializer_list.
     *
     * \throw khepri::ArgumentError if \a points is empty.
     * \throw khepri::ArgumentError if \a points is not sorted on \a x.
     * \throw khepri::ArgumentError if \a points contains duplicate \a x.
     */
    explicit StepInterpolator(std::initializer_list<Point> points)
        : StepInterpolator({points.begin(), points.end()})
    {
    }

    /**
     * \brief Constructs a new StepInterpolator from a sequence of points.
     *
     * \throw khepri::ArgumentError if \a points is empty.
     * \throw khepri::ArgumentError if \a points is not sorted on \a x.
     * \throw khepri::ArgumentError if \a points contains duplicate \a x.
     */
    explicit StepInterpolator(gsl::span<const Point> points);

    /// \see Interpolator::interpolate
    [[nodiscard]] double interpolate(double x) const noexcept override;

private:
    std::vector<Point> m_points;
};

/**
 * @brief An interpolator that linearly interpolates to the next value
 *
 * This interpolator conceptually draws a straight line between adjancent control points and returns
 * the \a y value of that line for given \a x.
 *
 * This is computationally a fairly cheap interpolator, but note that using this interpolator could
 * cause a "jerk" at control points because the resulting graph is only C⁰ continuous.
 */
class LinearInterpolator final : public Interpolator
{
public:
    /**
     * \brief Constructs a new LinearInterpolator from an \a initializer_list.
     *
     * \throw khepri::ArgumentError if \a points is empty.
     * \throw khepri::ArgumentError if \a points is not sorted on \a x.
     * \throw khepri::ArgumentError if \a points contains duplicate \a x.
     */
    explicit LinearInterpolator(std::initializer_list<Point> points)
        : LinearInterpolator({points.begin(), points.end()})
    {
    }

    /**
     * \brief Constructs a new LinearInterpolator from a sequence of points.
     *
     * \throw khepri::ArgumentError if \a points is empty.
     * \throw khepri::ArgumentError if \a points is not sorted on \a x.
     * \throw khepri::ArgumentError if \a points contains duplicate \a x.
     */
    explicit LinearInterpolator(gsl::span<const Point> points);

    /// \see Interpolator::interpolate
    [[nodiscard]] double interpolate(double x) const noexcept override;

private:
    std::vector<Point> m_points;
};

/**
 * @brief An interpolator that approximates a smooth interpolation to the next value
 *
 * This interpolator modifies the \see LinearInterpolator by modulating the straight line between
 * control points with the cosine function. The result is a line that smoothly approaches the
 * control point.
 *
 * This interpolator is computationally cheaper than the \see CubicInterpolator but is only C¹
 * continuous: it guarantees the tangent of the graph is the same on both sides of a control point
 * (the tangent is always 0 at control points).
 */
class CosineInterpolator final : public Interpolator
{
public:
    /**
     * \brief Constructs a new CosineInterpolator from an \a initializer_list.
     *
     * \throw khepri::ArgumentError if \a points is empty.
     * \throw khepri::ArgumentError if \a points is not sorted on \a x.
     * \throw khepri::ArgumentError if \a points contains duplicate \a x.
     */
    explicit CosineInterpolator(std::initializer_list<Point> points)
        : CosineInterpolator({points.begin(), points.end()})
    {
    }

    /**
     * \brief Constructs a new CosineInterpolator from a sequence of points.
     *
     * \throw khepri::ArgumentError if \a points is empty.
     * \throw khepri::ArgumentError if \a points is not sorted on \a x.
     * \throw khepri::ArgumentError if \a points contains duplicate \a x.
     */
    explicit CosineInterpolator(gsl::span<const Point> points);

    /// \see Interpolator::interpolate
    [[nodiscard]] double interpolate(double x) const noexcept override;

private:
    std::vector<Point> m_points;
};

/**
 * @brief An interpolator that creates a smooth interpolation from one control point to the next
 *
 * This interpolator creates a graph from a series of third-degree ("cubic") polynomials that
 * smoothly proceeds through the control points. Unlike the \see StepInterpolator, \see
 * LinearInterpolator and \see CosineInterpolator, the resulting graph may have local maxima and
 * minima that are not on the control points themselves.
 *
 * This interpolator is computationally the most expensive interpolator but guarantees C²
 * continuity: the tangent and curvature of the graph is the same on both sides of a control point.
 */
class CubicInterpolator final : public Interpolator
{
public:
    /**
     * \brief Constructs a new CubicInterpolator from an \a initializer_list.
     *
     * \throw khepri::ArgumentError if \a points is empty.
     * \throw khepri::ArgumentError if \a points is not sorted on \a x.
     * \throw khepri::ArgumentError if \a points contains duplicate \a x.
     */
    explicit CubicInterpolator(std::initializer_list<Point> points)
        : CubicInterpolator({points.begin(), points.end()})
    {
    }

    /**
     * \brief Constructs a new CubicInterpolator from a sequence of points.
     *
     * \throw khepri::ArgumentError if \a points is empty.
     * \throw khepri::ArgumentError if \a points is not sorted on \a x.
     * \throw khepri::ArgumentError if \a points contains duplicate \a x.
     */
    explicit CubicInterpolator(gsl::span<const Point> points);

    /// \see Interpolator::interpolate
    [[nodiscard]] double interpolate(double x) const noexcept override;

private:
    struct Segment
    {
        CubicPolynomial polynomial;
        double          min_x;
    };

    static std::vector<Segment> create_segments(gsl::span<const Point> points);

    std::vector<Segment> m_segments;

    // Copy of the input points
    std::vector<Point> m_points;
};

} // namespace khepri