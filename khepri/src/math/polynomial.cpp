#pragma once

#include <khepri/math/math.hpp>
#include <khepri/math/polynomial.hpp>

#include <cassert>

namespace khepri::detail {
namespace {

// Solve a constant function: y = c₀
std::vector<double> solve_constant(double y, gsl::span<const double> coefficients)
{
    // Solution: x = ℝ (set of all real numbers) iff. y = c₀
    assert(coefficients.size() == 1);
    if (is_near(y, coefficients[0])) {
        // Technically we should return all possible x, but that's not really doable.
        return {0.0};
    }
    return {};
}

// Solve a linear polynomial: y = c₀ + c₁·x
std::vector<double> solve_linear_polynomial(double y, gsl::span<const double> coefficients)
{
    // Solution: x = (y - c₀)/c₁
    assert(coefficients.size() == 2);
    return {(y - coefficients[0]) / coefficients[1]};
}

// Solve a quadratic polynomial: y = c₀ + c₁·x +  c₂·x²
std::vector<double> solve_quadratic_polynomial(double y, gsl::span<const double> coefficients)
{
    assert(coefficients.size() == 3);
    const auto a = coefficients[2], b = coefficients[1], c = coefficients[0] - y;
    // Solution: x = (-b ± √(b² - 4ac)) / 2a
    if (const auto D = b * b - 4 * a * c; D >= 0) {
        // There's a solution.
        const auto sqrt_D = std::sqrt(D);
        return {(-b - sqrt_D) / (2 * a), (-b + sqrt_D) / (2 * a)};
    }
    return {};
}

// Solve a cubic polynomial: y = c₀ + c₁·x + c₂·x² + c₃·x³
std::vector<double> solve_cubic_polynomial(double y, gsl::span<const double> coefficients)
{
    assert(coefficients.size() == 4);
    // First, rewrite to make c₃ = 1 (without loss of generality):
    // Cubic: (c₀-y)/c₃ + c₁/c₃·x + c₂/c₃·x² + x³ = 0
    const auto a0 = (coefficients[0] - y) / coefficients[3];
    const auto a1 = coefficients[1] / coefficients[3];
    const auto a2 = coefficients[2] / coefficients[3];

    // Then, the three solutions are (Cardano's formula):
    //   x₁ = -⅓a₂ + S + T
    //   x₂ = -⅓a₂ - ½(S + T) + ½√3𝑖(S - T)
    //   x₃ = -⅓a₂ - ½(S + T) - ½√3𝑖(S - T)
    // With:
    //   Q = (3a₁ - a₂²)/9,  R = (9a₂a₁ - 27a₀ - 2a₂³)/54,
    //   S = ∛(R + √D),  T = ∛(R - √D), and D = Q³ + R²

    const auto Q = (3 * a1 - a2 * a2) / 9;
    const auto R = (9 * a2 * a1 - 27 * a0 - 2 * a2 * a2 * a2) / 54;
    const auto D = Q * Q * Q + R * R;
    if (is_near(D, 0.0)) {
        // If D is zero, S and T are equal, the imaginary component in x₂ and x₃
        // disappears and they become equal. So then we have two real solutions.
        const auto S = std::cbrt(R);

        auto x1 = 2 * S - a2 / 3;
        auto x2 = -S - a2 / 3;
        if (is_near(x1, x2)) {
            return {x1};
        }
        // Sort the result
        if (x2 < x1) {
            std::swap(x1, x2);
        }
        return {x1, x2};
    }

    if (D > 0) {
        // If D is positive, S and T are different real numbers, and x₂ and x₃ become
        // complex numbers. We only want real solutions, so we ignore them.
        const auto sqrt_D = std::sqrt(D);
        const auto S      = std::cbrt(R + sqrt_D);
        const auto T      = std::cbrt(R - sqrt_D);
        return {S + T - a2 / 3};
    }

    // If D is negative, S and T are complex numbers. These eventually factor out to result
    // in three real numbers as follows:
    //   θ  = cos⁻¹(R / √(-Q³))
    //   x₁ = -⅓a₂ + 2√(-Q)cos(θ/3)
    //   x₂ = -⅓a₂ + 2√(-Q)cos((θ+2π)/3)
    //   x₃ = -⅓a₂ + 2√(-Q)cos((θ+4π)/3)
    assert(Q <= 0);
    const auto theta  = std::acos(R / std::sqrt(-Q * Q * Q));
    const auto sqrt_Q = 2 * std::sqrt(-Q);

    auto x1 = sqrt_Q * std::cos(theta / 3) - a2 / 3;
    auto x2 = sqrt_Q * std::cos((theta + 2 * PI) / 3) - a2 / 3;
    auto x3 = sqrt_Q * std::cos((theta + 4 * PI) / 3) - a2 / 3;
    // Sort the result
    if (x2 < x1) {
        std::swap(x1, x2);
    }
    if (x3 < x2) {
        std::swap(x3, x2);
        if (x2 < x1) {
            std::swap(x1, x2);
        }
    }
    return {x1, x2, x3};
}

// Solve a quartic polynomial: y = c₀ + c₁·x + c₂·x² + c₃·x³ + c₄·x⁴
std::vector<double> solve_quartic_polynomial(double y, gsl::span<const double> coefficients)
{
    assert(coefficients.size() == 5);

    // Apply the quartic formula.
    // First normalize the quartic into a monic by dividing by c₄: x⁴ + bx³ + cx² + dx + e = 0, with
    // b = c₃/c₄, c = c₂/c₄, d = c₁/c₄ and e = (c₀-y)/c₄
    const auto b = coefficients[3] / coefficients[4], c = coefficients[2] / coefficients[4],
               d = coefficients[1] / coefficients[4], e = (coefficients[0] - y) / coefficients[4];

    // Then, solve the resolvent cubic: z³ - cz² + (db - 4e)z + (4ce - d² - b²e) = 0
    const auto cubic_coefficients =
        std::array<double, 4>{4 * c * e - d * d - b * b * e, d * b - 4 * e, -c, 1};
    const auto zs = solve_cubic_polynomial(0, cubic_coefficients);
    if (zs.empty()) {
        // No solutions
        return {};
    }

    // Use a non-zero real root of the cubic (largest for better precision)
    auto it = std::find_if(zs.rbegin(), zs.rend(), [](auto v) { return !is_near(v, 0); });
    if (it == zs.rend()) {
        // The resolvent cubic only has a single solution: 0.
        // That means the logic below won't work nicely, but we have a single solution anyway.
        return {0};
    }
    const auto z = *it;

    // Then, calculate the solutions to the monic quartic
    const auto R = std::sqrt(b * b / 4 - c + z) / 2;
    const auto m = (b * b * 3 / 16 - R * R - c / 2);
    const auto n =
        is_near(R, 0) ? std::sqrt(z * z / 4 - e) : (b * c / 8 - d / 4 - b * b * b / 32) / R;

    std::vector<double> xs;
    if (m + n >= 0) {
        const auto D = std::sqrt(m + n);
        xs.push_back(b / -4 + R + D);
        if (!is_near(D, 0)) {
            xs.push_back(b / -4 + R - D);
        }
    }

    if (m >= n) {
        const auto E = std::sqrt(m - n);
        xs.push_back(b / -4 - R + E);
        if (!is_near(E, 0)) {
            xs.push_back(b / -4 - R - E);
        }
    }

    std::sort(xs.begin(), xs.end());
    return xs;
}

} // namespace

std::vector<double> solve_polynomial(double y, gsl::span<const double> coefficients)
{
    // By the Abel-Ruffini theorem, there are only exact solutions for polynomials up to
    // degree 4. Numerical root-finding methods are quite complicated, and can be very sensitive to
    // errors in floating-point arithmethic (see Wilkinson's Polynomial). They'd need
    // arbitrary-precision decimal math types. This is simply too much, and the need for games to
    // solve fifth-degree-or-higher polynomials is quite low, so we just don't support it.
    assert(coefficients.size() >= 1 && coefficients.size() < 5);

    // The number of coefficients is always 1 + the polynomial degree.
    const std::size_t degree = coefficients.size() - 1;

    if (degree >= 5) {
        return {};
    }

    // The exact solutions require the highest coefficient to not be zero. Otherwise, it's really a
    // lesser-degree polynomial. That's why we check the coefficients here.

    if (degree >= 4 && !is_near(coefficients[4], 0.0)) {
        return solve_quartic_polynomial(y, coefficients.subspan(0, 5));
    }

    if (degree >= 3 && !is_near(coefficients[3], 0.0)) {
        return solve_cubic_polynomial(y, coefficients.subspan(0, 4));
    }

    if (degree >= 2 && !is_near(coefficients[2], 0.0)) {
        return solve_quadratic_polynomial(y, coefficients.subspan(0, 3));
    }

    if (degree >= 1 && !is_near(coefficients[1], 0.0)) {
        return solve_linear_polynomial(y, coefficients.subspan(0, 2));
    }

    return solve_constant(y, coefficients.subspan(0, 1));
}

} // namespace khepri::detail
