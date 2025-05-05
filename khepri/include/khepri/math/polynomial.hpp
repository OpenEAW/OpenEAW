#pragma once

#include "point.hpp"

#include <gsl/gsl-lite.hpp>

#include <array>
#include <type_traits>
#include <vector>

namespace khepri {

/**
 * \brief A generic N-degree polynomial
 *
 * n-degree polynomials (with n >= 0) are defined as: f(x) = c₀ + c₁·x + c₂·x² + ... + cₙ·xⁿ
 * They are defined by their coeffiecients (c₀, c₁, c₂, ..., cₙ).
 *
 * \note Polynomial<0> is a constant function. \a x is ignored during sampling.
 */
template <std::size_t Degree>
struct Polynomial
{
    /// The coefficients of polynomial
    std::array<double, Degree + 1> coefficients;

    /**
     * @brief Samples the polynomial for given \a x.
     */
    [[nodiscard]] double sample(double x) const noexcept
    {
        // Use Horner's rule for polynomial evaluation
        double y = coefficients[Degree];
        for (std::size_t i = Degree; i >= 1; --i) {
            y = coefficients[i - 1] + x * y;
        }
        return y;
    }

    /**
     * @brief Returns the derivative polynomial of the polynomial.
     *
     * The derivative polynomial is always one degree less than the polynomial it is derived from,
     * except for a polynomial of degree 0: it's derivative is also a 0-degree polynomial (f(x) = 0
     * to be precise).
     */
    Polynomial<std::max<std::size_t>(Degree, 1) - 1> derivative() const noexcept
    {
        if constexpr (Degree == 0) {
            // The derivative of a constant is 0.
            return {{0}};
        } else {
            Polynomial<Degree - 1> p;
            for (std::size_t i = 1; i <= Degree; ++i) {
                p.coefficients[i - 1] = i * coefficients[i];
            }
            return p;
        }
    }
};

/**
 * @brief A first-degree (linear) polynomial
 *
 * Linear polynomials are defined as y = a + b·x.
 */
struct LinearPolynomial : public Polynomial<1>
{};

/**
 * @brief A second-degree (quadratic) polynomial
 *
 * Quadratic polynomials are defined as y = a + b·x + c·x².
 */
struct QuadraticPolynomial : public Polynomial<2>
{};

/**
 * @brief A third-degree (cubic) polynomial
 *
 * Cubic polynomials are defined as y = a + b·x + c·x² + d·x³.
 */
struct CubicPolynomial : public Polynomial<3>
{};

} // namespace khepri