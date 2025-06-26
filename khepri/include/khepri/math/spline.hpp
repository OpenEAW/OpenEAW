#pragma once

#include "vector3.hpp"

#include <gsl/gsl-lite.hpp>

#include <initializer_list>
#include <vector>

namespace khepri {

/**
 * @brief A cubic spline
 *
 * A cubic spline is a spline that follows a set of predefined control points.
 * By defining third-order (cubic) polynomial segments between every adjacent set of control points,
 * the resulting curve runs smoothly through every control point. This line can be interpolated at
 * any point from start to finish. Sampling at regular intervals is guaranteed to return points
 * that are at identical distances along the curve.
 */
class CubicSpline
{
public:
    /**
     * @brief Construct a new CubicSpline.
     *
     * @param points the points through which the spline must traverse.
     *
     * @throws khepri::ArgumentError if points.size() < 2
     */
    explicit CubicSpline(gsl::span<const Vector3> points);

    /**
     * @brief Construct a new CubicSpline.
     *
     * @param points the points through which the spline must traverse.
     *
     * @throws khepri::ArgumentError if points.size() < 2
     */
    explicit CubicSpline(std::initializer_list<Vector3> points);

    /**
     * @brief Returns the points that this spline was constructed with.
     *
     * @note the returned span is valid until the spline is destroyed.
     */
    [[nodiscard]] gsl::span<const Vector3> points() const noexcept
    {
        return m_points;
    }

    /**
     * @brief Returns the length of the spline as measured along its curve.
     */
    [[nodiscard]] double length() const noexcept
    {
        return m_arc_offsets.back();
    }

    /**
     * @brief Returns the length of the spline at one of its points.
     * @param point_index index of the point. Must be in the range <em>[0, points().size())</em>
     */
    [[nodiscard]] double length_at(std::size_t point_index) const noexcept;

    /**
     * @brief Samples the spline at fractional offset @a t along the spline
     *
     * The spline is sampled at offset @a t, which is a fraction indicating how far along the spline
     * it should be sampled.
     *
     * This class guarantees uniform sampling in arc length. This means that multiple samples with a
     * constant interval in @a t results in samples which have a constant arc-length distance along
     * the spline. This allows for e.g. movement along the spline with constant speed.
     *
     * @param t the position along the spline from 0.0 to 1.0.
     */
    [[nodiscard]] Vector3 sample(double t) const noexcept;

private:
    // Definition of a cubic polynomial defined as y = a + b*x + c*x^2 + d*x^3.
    // It is valid for x in [0,1].
    struct Polynomial
    {
        double a, b, c, d;

        [[nodiscard]] double sample(double x) const noexcept;
    };

    class Polynomials
    {
    public:
        explicit Polynomials(gsl::span<const Vector3> points);

        [[nodiscard]] auto size() const noexcept
        {
            return m_polynomials_x.size();
        }

        /**
         * @brief Samples a polynomial with input coordinate u.
         *
         * @param index     polynomial to sample
         * @param u_from    input coordinate to sample the polynomial at.
         */
        [[nodiscard]] Vector3 sample(std::size_t index, double u) const noexcept;

    private:
        static std::vector<Polynomial> calculate_polynomials(gsl::span<const double> points);

        // The n-1 parameterized polynomials for each dimension of the spline.
        // The polynomials are uniformly parameterized on [0,1].
        std::vector<Polynomial> m_polynomials_x;
        std::vector<Polynomial> m_polynomials_y;
        std::vector<Polynomial> m_polynomials_z;
    };

    [[nodiscard]] static std::vector<double>
    calculate_arc_offsets(const Polynomials& polynomials) noexcept;

    /**
     * @brief Find the arc length of a curve segment between two points along the curve
     *
     * @param index     polynomial to get the arc length for.
     * @param u_from    input coordinate for the left side of the curve segment.
     * @param u_to      input coordinate for the right side of the curve segment.
     */
    [[nodiscard]] static double arc_length(const Polynomials& polynomials, std::size_t index,
                                           double u_from, double u_to) noexcept;

    Polynomials m_polynomials;

    // (Approximated) arc offsets from the start of the spline to the end of each polynomial
    std::vector<double> m_arc_offsets;

    // Copy of the input points
    std::vector<Vector3> m_points;
};

} // namespace khepri
