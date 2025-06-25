#include <khepri/exceptions.hpp>
#include <khepri/math/interpolator.hpp>
#include <khepri/math/math.hpp>
#include <khepri/utility/functional.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

namespace khepri {
namespace {
void check_sorted(gsl::span<const Point> points)
{
    if (points.empty()) {
        throw ArgumentError();
    }

    for (std::size_t i = 1; i < points.size(); ++i) {
        if (points[i].x <= points[i - 1].x) {
            throw ArgumentError();
        }
    }
}

// Checks if two floating-point values are near enough to be considered equal
bool is_near(double v1, double v2) noexcept
{
    static constexpr double MAX_DIFF = 0.00000001;
    return std::abs(v1 - v2) < MAX_DIFF;
}

/// Returns the index of the last point that has an \a x member less than \a x.
std::size_t find_index(gsl::span<const Point> points, double x)
{
    // x must be in the range of the points
    assert(!points.empty());
    assert(x >= points.begin()->x && x <= points.rbegin()->x);

    const auto* it = std::upper_bound(points.begin(), points.end(), x,
                                      [&](double value, auto& item) { return value < item.x; });
    if (it == points.begin()) {
        // This shouldn't happen because x must be clamped to the points range, but perhaps
        // it's possible due to floating-point comparison weirdness. Anyway, we meant to
        // hit the next one.
        std::advance(it, 1);
    }

    // Return the index to the point on the _left_ of \a x such that
    // \a x is greater-than-or-equal to the returned point.
    return std::distance(points.begin(), std::prev(it));
}
} // namespace

StepInterpolator::StepInterpolator(gsl::span<const Point> points)
{
    check_sorted(points);
    m_points = {points.begin(), points.end()};
}

double StepInterpolator::interpolate(double x) const noexcept
{
    x = clamp(x, m_points.front().x, m_points.back().x);

    const auto index = find_index(m_points, x);
    return m_points[index].y;
}

LinearInterpolator::LinearInterpolator(gsl::span<const Point> points)
{
    check_sorted(points);
    m_points = {points.begin(), points.end()};
}

double LinearInterpolator::interpolate(double x) const noexcept
{
    x = clamp(x, m_points.front().x, m_points.back().x);

    const auto index = find_index(m_points, x);

    x = x - m_points[index].x;
    if ((index == m_points.size() - 1) || is_near(x, 0.0)) {
        return m_points[index].y;
    }

    // If dx were 0, then x would have to be near the first point, and the condition above would
    // prevent us from reaching this.
    const double dx = m_points[index + 1].x - m_points[index].x;
    const double dy = m_points[index + 1].y - m_points[index].y;
    assert(!is_near(dx, 0.0));

    x = x / dx;
    return m_points[index].y + dy * x;
}

CosineInterpolator::CosineInterpolator(gsl::span<const Point> points)
{
    check_sorted(points);
    m_points = {points.begin(), points.end()};
}

double CosineInterpolator::interpolate(double x) const noexcept
{
    x = clamp(x, m_points.front().x, m_points.back().x);

    const auto index = find_index(m_points, x);

    x = x - m_points[index].x;
    if ((index == m_points.size() - 1) || is_near(x, 0.0)) {
        return m_points[index].y;
    }

    const double dx = m_points[index + 1].x - m_points[index].x;
    const double dy = m_points[index + 1].y - m_points[index].y;
    assert(!is_near(dx, 0.0));

    x = x / dx;
    x = (1 - std::cos(x * PI)) / 2;
    return m_points[index].y + dy * x;
}

CubicInterpolator::CubicInterpolator(gsl::span<const Point> points)
{
    check_sorted(points);
    m_segments = create_segments(points);
    m_points   = {points.begin(), points.end()};
}

std::vector<CubicInterpolator::Segment>
CubicInterpolator::create_segments(gsl::span<const Point> points)
{
    assert(!points.empty());

    if (points.size() == 1) {
        // Horizontal line at y = points[0].y
        return {{{points[0].y, 0, 0, 0}, 0}};
    }

    if (points.size() == 2) {
        // Straight line from points[0] to points[1]
        return {{{points[0].y, (points[1].y - points[0].y) / (points[1].x - points[0].x), 0, 0},
                 points[0].x}};
    }

    /*
     * Define N cubic polynomials (fᵢ(x) for 0 ≤ i < N) that interpolate the N+1 points, where
     * xᵢ = points[i].x, with x₀ < x₁ < ... < xₙ (i.e. x is incrementing), with their first and
     * second derivatives:
     *
     *   fᵢ(x)   = Aᵢ + Bᵢ·(x-xᵢ) + Cᵢ·(x-xᵢ)² + Dᵢ·(x-xᵢ)³                (1)
     *   f′ᵢ(x)  = Bᵢ + 2·Cᵢ·(x-xᵢ) + 3·Dᵢ·(x-xᵢ)²                         (2)
     *   f′′ᵢ(x) = 2·Cᵢ + 6·Dᵢ·(x-xᵢ)                                      (3)
     *
     * where fᵢ is valid for the domain [xᵢ, xᵢ₊₁]. To find the 4·N coefficients of these
     * polynomials, we need 4·N equations. We start by constraining f to interpolate `points` (eq.
     * 4) and be C² continuous on every point (position, tangent and curvature match; eqs. 5–7):
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
     *
     * To solve the rest, we first solve for C. For convenience, let hᵢ = xᵢ₊₁ - xᵢ and substitute
     * eqs. 1–3 into eqs. 5–7:
     *
     *  Bᵢ·hᵢ +  Cᵢ·hᵢ² +   Dᵢ·hᵢ³ = Aᵢ₊₁ - Aᵢ                             (10)
     *         2·Cᵢ·hᵢ  + 3·Dᵢ·hᵢ² = Bᵢ₊₁ - Bᵢ                             (11)
     *                    3·Dᵢ·hᵢ  = Cᵢ₊₁ - Cᵢ                             (12)
     *
     * Rewrite eq. 12 for Dᵢ and substitute into eqs. 10 and 11:
     *
     *                             Dᵢ = (Cᵢ₊₁ - Cᵢ)/(3·hᵢ)                 (13)
     *  Bᵢ·hᵢ + ⅓·Cᵢ₊₁·hᵢ² + ⅔·Cᵢ·hᵢ² = Aᵢ₊₁ - Aᵢ                          (14)
     *            Cᵢ₊₁·hᵢ  +   Cᵢ·hᵢ  = Bᵢ₊₁ - Bᵢ                          (15)
     *
     * Rewrite eq. 14 for Bᵢ and Bᵢ₊₁:
     *
     *   Bᵢ   = (Aᵢ₊₁ - Aᵢ)/hᵢ     - ⅓·Cᵢ₊₁·hᵢ   - ⅔·Cᵢ·hᵢ                 (16)
     *   Bᵢ₊₁ = (Aᵢ₊₂ - Aᵢ₊₁)/hᵢ₊₁ - ⅓·Cᵢ₊₂·hᵢ₊₁ - ⅔·Cᵢ₊₁·hᵢ₊₁             (17)
     *
     * Substitute eqs. 16 and 17 into eq. 15 and rewrite into:
     *
     *   hᵢ·Cᵢ + 2·(hᵢ + hᵢ₊₁)·Cᵢ₊₁ + hᵢ₊₁·Cᵢ₊₂ = 3·(Aᵢ₊₂ - Aᵢ₊₁)/hᵢ₊₁ - 3·(Aᵢ₊₁ - Aᵢ)/hᵢ
     *                                                    for 0 ≤ i ≤ N-2  (18)
     *
     * Shift the index downward by one to get
     *
     *   hᵢ₋₁·Cᵢ₋₁ + 2·(hᵢ₋₁ + hᵢ)·Cᵢ + hᵢ·Cᵢ₊₁ = 3·(Aᵢ₊₁ - Aᵢ)/hᵢ - 3·(Aᵢ - Aᵢ₋₁)/hᵢ₋₁
     *                                                    for 1 ≤ i ≤ N-1  (19)
     *
     * Note that h and A are known. From eqs. 2, 3, 8 and 9 we additionally get:
     *
     *   C₀  = 0                                                           (20)
     *   Cₙ₋₁ = 0                                                           (21)
     *
     * Eqs. 19–21 describe N+1 linear equations that can be expressed in matrix form:
     *
     *   [1  0  0  0    0   0   ]  [C₀ ]   [                   0                    ]
     *   [s₁ d₁ u₁ 0    0   0   ]  [C₁ ]   [   3·(A₂ - A₁)/h₁  - 3·(A₁ - A₀)/h₀     ]
     *   [0  s₂ d₂ u₂   0   0   ]  [C₂ ]   [   3·(A₃ - A₂)/h₂  - 3·(A₂ - A₁)/h₁     ]
     *   [         ...          ]  [...] = [                  ...                   ]
     *   [0  0  0  sₙ₋₁ dₙ₋₁ uₙ₋₁]  [Cₙ₋₁]   [ 3·(Aₙ - Aₙ₋₁)/hₙ₋₁ - 3·(Aₙ₋₁ - Aₙ₋₂/hₙ₋₂)]
     *   [0  0  0  0    0   1   ]  [Cₙ  ]   [                   0                    ]
     *
     * where sᵢ = hᵢ₋₁, dᵢ = 2·(hᵢ₋₁ + hᵢ) and uᵢ = hᵢ
     *
     * Or M · C = A, where the matrix M and vector A are known and C is the vector of Cᵢ components.
     * Solving for C becomes C = M⁻¹ · A. Because dᵢ ≥ sᵢ + uᵢ, M is a diagonally dominant
     * tridiagonal matrix so solving for C can be done via Thomas' algorithm.
     *
     * With A and C known, eqs. 10 and 12 can be used to find B and D.
     */

    // Run Thomas' algorithm

    // The new superdiagonal elements
    std::vector<double> superd(points.size() - 1);

    // The result elements of the expression (the knowns).
    // This will also hold the computed unknowns during the backpropagation step.
    std::vector<double> result(points.size());

    superd.front() = result.front() = 0;
    for (std::size_t i = 1; i < superd.size(); ++i) {
        const double alpha = 3 * (points[i + 1].y - points[i].y) / (points[i + 1].x - points[i].x) -
                             3 * (points[i].y - points[i - 1].y) / (points[i].x - points[i - 1].x);
        const double tmp = 2 * (points[i + 1].x - points[i - 1].x) -
                           superd[i - 1] * (points[i].x - points[i - 1].x);

        superd[i] = (points[i + 1].x - points[i].x) / tmp;
        result[i] = (alpha - (points[i].x - points[i - 1].x) * result[i - 1]) / tmp;
    }

    // Back-substitute to find the unknown vector with coefficient C for the polynomials
    result.back() = 0;
    for (std::size_t i = result.size() - 1; i > 0; --i) {
        result[i - 1] -= superd[i - 1] * result[i];
    }

    // Construct the polynomial segments from the coefficients a, b, c and d.
    std::vector<Segment> segments(points.size() - 1);
    for (std::size_t i = 0; i < segments.size(); ++i) {
        const double h = (points[i + 1].x - points[i].x);
        const double a = points[i].y;
        const double b =
            (points[i + 1].y - points[i].y) / h - (result[i + 1] + 2 * result[i]) * h / 3;
        const double d = (result[i + 1] - result[i]) / (3 * h);
        segments[i]    = {{a, b, result[i], d}, points[i].x};
    }
    return segments;
}

double CubicInterpolator::interpolate(double x) const noexcept
{
    x = clamp(x, m_points.front().x, m_points.back().x);

    const auto index = find_index(m_points, x);

    if ((index == m_points.size() - 1) || is_near(x, m_points[index].x)) {
        return m_points[index].y;
    }
    const auto& segment = m_segments[index];

    return segment.polynomial.sample(x - segment.min_x);
}

} // namespace khepri