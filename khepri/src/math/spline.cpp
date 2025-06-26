#include <khepri/exceptions.hpp>
#include <khepri/math/math.hpp>
#include <khepri/math/spline.hpp>
#include <khepri/utility/functional.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>

namespace khepri {

double CubicSpline::Polynomial::sample(double x) const noexcept
{
    assert(x >= 0 && x <= 1);
    return a + (b + (c + d * x) * x) * x;
}

std::vector<CubicSpline::Polynomial>
CubicSpline::Polynomials::calculate_polynomials(gsl::span<const double> points)
{
    if (points.size() < 2) {
        throw khepri::ArgumentError();
    }

    if (points.size() == 2) {
        // Straight line of (0,points[0]) to (1,points[1])
        return {{points[0], points[1] - points[0], 0, 0}};
    }

    /*
     * Define N cubic polynomials (fᵢ(x) for 0 ≤ i < N) that interpolate the N+1 points, where
     * points[i] is assumed to be the value for xᵢ, with x₀ < x₁ < ... < xₙ (i.e. x is
     * incrementing), with their first and second derivatives:
     *
     *   fᵢ(x)   = Aᵢ + Bᵢ·(x-xᵢ) + Cᵢ·(x-xᵢ)² + Dᵢ·(x-xᵢ)³                (1)
     *   f′ᵢ(x)  = Bᵢ + 2·Cᵢ·(x-xᵢ) + 3·Dᵢ·(x-xᵢ)²                         (2)
     *   f′′ᵢ(x) = 2·Cᵢ + 6·Dᵢ·(x-xᵢ)                                      (3)
     *
     * where fᵢ is valid for the domain [xᵢ, xᵢ₊₁]. To solve the 4·N coefficients of these
     * polynomials, we need 4·N equations. We start by constraining f to interpolate `points` (eq.
     * 1) and be C² continuous on every point (position, tangent and curvature match; eqs. 2–4):
     *
     *   fᵢ(xᵢ)     = points[i]     for 0 ≤ i < N                          (4)
     *   fᵢ(xᵢ₊₁)   = fᵢ₊₁(xᵢ₊₁)    for 0 ≤ i < N                          (5)
     *   f′ᵢ(xᵢ₊₁)  = f′ᵢ₊₁(xᵢ₊₁)   for 0 ≤ i < N-1                        (6)
     *   f′′ᵢ(xᵢ₊₁) = f′′ᵢ₊₁(xᵢ₊₁)  for 0 ≤ i < N-1                        (7)
     *
     * This yields 4·N-2 equations. We also use _natural_ cubic splines: the end points have zero
     * curvature. This gives us 2 additional equations for the total 4·N:
     *
     *   f′′₀ (x₀) = 0                                                     (8)
     *   f′′ₙ₋₁(xₙ) = 0                                                     (9)
     *
     * Solving Aᵢ is trivial: eq. 4 simplifies to Aᵢ = points[i].
     * From eqs. 2–3, it follows that:
     *
     *   f′ᵢ(xᵢ)  =   Bᵢ
     *   f′′ᵢ(xᵢ) = 2·Cᵢ
     *
     * Let xᵢ₊₁ - xᵢ = 1 (i.e. assume the points are uniformly sampled at x = [0,1, ..., N]) and
     * substitute eqs. 1–3 into eqs 5–7:
     *
     *   Aᵢ + Bᵢ +   Cᵢ +   Dᵢ = Aᵢ₊₁                                      (10)
     *        Bᵢ + 2·Cᵢ + 3·Dᵢ = Bᵢ₊₁                                      (11)
     *               Cᵢ + 3·Dᵢ = Cᵢ₊₁                                      (12)
     *
     * Rewrite eq. 12 for Dᵢ and substitute into eqs. 10 and 11:
     *
     *                   Dᵢ = ⅓·(Cᵢ₊₁ - Cᵢ)                                (13)
     *   Bᵢ + ⅓·Cᵢ₊₁ + ⅔·Cᵢ =    Aᵢ₊₁ - Aᵢ                                 (14)
     *          Cᵢ₊₁ +   Cᵢ =    Bᵢ₊₁ - Bᵢ                                 (15)
     *
     * Rewrite eq. 14 for Bᵢ and Bᵢ₊₁:
     *
     *   Bᵢ   = Aᵢ₊₁ - Aᵢ   - ⅓·(Cᵢ₊₁ + 2·Cᵢ  )                            (16)
     *   Bᵢ₊₁ = Aᵢ₊₂ - Aᵢ₊₁ - ⅓·(Cᵢ₊₂ + 2·Cᵢ₊₁)                            (17)
     *
     * Substitute eqs. 16 and 17 into eq. 15 and rewrite into:
     *
     *   Cᵢ + 4·Cᵢ₊₁ + Cᵢ₊₂ = 3·(Aᵢ - 2·Aᵢ₊₁ + Aᵢ₊₂)    for 0 ≤ i ≤ N-2    (18)
     *
     * Shift the index downward by one to get
     *
     *   Cᵢ₋₁ + 4·Cᵢ + Cᵢ₊₁ = 3·(Aᵢ₋₁ - 2·Aᵢ + Aᵢ₊₁)    for 1 ≤ i ≤ N-1    (19)
     *
     * Note that A is known. From eqs. 2, 3, 8 and 9 we additionally get:
     *
     *   C₀  = 0                                                           (20)
     *   Cₙ₋₁ = 0                                                           (21)
     *
     * Eqs. 19–21 describe N+1 linear equations that can be expressed in matrix form:
     *
     *   [1 0 0 0 0 0]  [C₀ ]   [          0          ]
     *   [1 4 1 0 0 0]  [C₁ ]   [  3·(A₀ - 2·A₁ + A₂) ]
     *   [0 1 4 1 0 0]  [C₂ ]   [  3·(A₁ - 2·A₂ + A₃) ]
     *   [    ...    ]  [...] = [          ...        ]
     *   [0 0 0 1 4 1]  [Cₙ₋₁]   [3·(Aₙ₋₂ - 2·Aₙ₋₁ + Aₙ)]
     *   [0 0 0 0 0 1]  [Cₙ  ]   [           0         ]
     *
     * Or M · C = A, where the matrix M and vector A are known and C is the vector of Cᵢ components.
     * Solving for C becomes C = M⁻¹ · A. M is a diagonally dominant tridiagonal matrix so
     * calculating M⁻¹ can be done via Thomas' algorithm.
     *
     * With A and C known, eqs. 10 and 12 can be used to find D and B.
     */

    // Run Thomas' algorithm

    // The new superdiagonal elements
    std::vector<double> superd(points.size() - 1);

    // The result elements of the expression (the knowns).
    // This will also hold the computed unknowns during the backpropagation step.
    std::vector<double> result(points.size());

    superd.front() = result.front() = 0;
    for (std::size_t i = 1; i < superd.size(); ++i) {
        const double result_i = 3 * (points[i - 1] - 2 * points[i] + points[i + 1]);
        superd[i]             = 1.0 / (4 - superd[i - 1]);
        result[i]             = (result_i - result[i - 1]) * superd[i];
    }

    // Back-substitute to find the unknown vector with coefficient C for the polynomials
    result.back() = 0;
    for (std::size_t i = result.size() - 1; i > 0; --i) {
        result[i - 1] -= superd[i - 1] * result[i];
    }

    // Construct the polynomials from the coefficients a, b, c and d.
    std::vector<CubicSpline::Polynomial> polynomials(points.size() - 1);
    for (std::size_t i = 0; i < polynomials.size(); ++i) {
        const double a = points[i];
        const double d = (result[i + 1] - result[i]) / 3.0;
        const double b = points[i + 1] - points[i] - result[i] - d;
        polynomials[i] = {a, b, result[i], d};
    }
    return polynomials;
}

CubicSpline::Polynomials::Polynomials(gsl::span<const Vector3> points)
    : m_polynomials_x(calculate_polynomials(pluck(points, &Vector3::x)))
    , m_polynomials_y(calculate_polynomials(pluck(points, &Vector3::y)))
    , m_polynomials_z(calculate_polynomials(pluck(points, &Vector3::z)))
{
}

Vector3 CubicSpline::Polynomials::sample(std::size_t index, double u) const noexcept
{
    assert(m_polynomials_x.size() == m_polynomials_y.size());
    assert(m_polynomials_y.size() == m_polynomials_z.size());
    assert(index < m_polynomials_x.size());
    assert(u >= 0 && u <= 1);

    const auto x = m_polynomials_x[index].sample(u);
    const auto y = m_polynomials_y[index].sample(u);
    const auto z = m_polynomials_z[index].sample(u);
    return {x, y, z};
}

CubicSpline::CubicSpline(gsl::span<const Vector3> points)
    : m_polynomials(points)
    , m_arc_offsets(calculate_arc_offsets(m_polynomials))
    , m_points{points.begin(), points.end()}
{
}

CubicSpline::CubicSpline(std::initializer_list<Vector3> points)
    : CubicSpline({points.begin(), points.end()})
{
}

double CubicSpline::arc_length(const Polynomials& polynomials, std::size_t index, double u_from,
                               double u_to) noexcept
{
    // This method finds the arc length by recursive division:
    // We divide the curve in two (half-way between u_from and u_to) and get the
    //
    assert(u_from <= u_to);
    assert(u_from >= 0.0 && u_to <= 1.0);

    constexpr auto min_accuracy = 0.00001;

    // Sample the cubic at the two end points
    const auto v_from = polynomials.sample(index, u_from);
    const auto v_to   = polynomials.sample(index, u_to);

    // Get the chordal length of the curve between the two endpoints
    const auto length = khepri::distance(v_from, v_to);
    if (length < min_accuracy) {
        // The length is too small, don't bother subdividing
        return length;
    }

    // Find and sample the mid-way point
    const auto u_mid = (u_from + u_to) / 2;
    const auto v_mid = polynomials.sample(index, u_mid);

    // Get the 'better' (more accurate) length by adding the chordal distances from the end points
    // to the mid-way point. This is typically larger than the direct length (because it's a curve).
    const auto better_length = khepri::distance(v_from, v_mid) + khepri::distance(v_mid, v_to);

    // Calculate relative and absolute error
    const auto rel_error = std::abs(length / better_length - 1);
    const auto abs_error = std::abs(length - better_length);

    if (rel_error < min_accuracy || abs_error < min_accuracy) {
        // Both errors are acceptably small, stop recursing
        return better_length;
    }

    // Recurse to add the arc lengths
    return arc_length(polynomials, index, u_from, u_mid) +
           arc_length(polynomials, index, u_mid, u_to);
}

std::vector<double> CubicSpline::calculate_arc_offsets(const Polynomials& polynomials) noexcept
{
    std::vector<double> arc_offsets(polynomials.size());
    arc_offsets[0] = arc_length(polynomials, 0, 0.0, 1.0);
    for (std::size_t i = 1; i < arc_offsets.size(); ++i) {
        arc_offsets[i] = arc_offsets[i - 1] + arc_length(polynomials, i, 0.0, 1.0);
    }
    return arc_offsets;
}

[[nodiscard]] double CubicSpline::length_at(std::size_t point_index) const noexcept
{
    assert(point_index <= m_arc_offsets.size());
    if (point_index == 0) {
        return 0.0;
    }
    return m_arc_offsets[point_index - 1];
}

Vector3 CubicSpline::sample(double t) const noexcept
{
    t = clamp(t, 0.0, 1.0);

    // t is in arc length, find the segment that belongs to it
    const auto spline_length = m_arc_offsets.back();
    const auto arc_offset    = t * spline_length;
    const auto index         = std::min<std::size_t>(
        std::distance(m_arc_offsets.begin(),
                              std::upper_bound(m_arc_offsets.begin(), m_arc_offsets.end(), arc_offset)),
        m_arc_offsets.size() - 1);

    auto arc_offset_segment = (index > 0 ? m_arc_offsets[index - 1] : 0.0);

    // Now find the uniform parameter for that segment
    auto u_start          = 0.0;
    auto arc_offset_start = arc_offset_segment;
    auto u_end            = 1.0;
    auto arc_offset_end   = m_arc_offsets[index];

    // Bound the iterations so we don't end up in an infinite loop due to some floating point
    // rounding oddity.
    constexpr double epsilon        = 0.0000001;
    constexpr int    max_iterations = 100;
    double           u              = u_start;
    if (arc_offset_end - arc_offset_start > epsilon) {
        // Non-degenerate segment
        for (int i = 0; i < max_iterations; ++i) {
            // Don't use midway interpolation, but linear interpolation based on the desired arc
            // offset. This has a higher chance of `u` already being in the right area and thus
            // needing fewer iterations.
            const auto frac = (arc_offset - arc_offset_start) / (arc_offset_end - arc_offset_start);
            u               = lerp(u_start, u_end, frac);
            const auto length = arc_length(m_polynomials, index, 0.0, u) + arc_offset_segment;
            if (std::abs(length - arc_offset) < epsilon) {
                // We're close enough to the desired arc offset
                break;
            }
            if (length < arc_offset) {
                u_start          = u;
                arc_offset_start = length;
            } else {
                u_end          = u;
                arc_offset_end = length;
            }
        }
    }

    return m_polynomials.sample(index, u);
}

} // namespace khepri
