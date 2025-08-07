#include "printers.hpp"

#include <khepri/math/polynomial.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using khepri::CubicPolynomial;
using khepri::LinearPolynomial;
using khepri::Polynomial;
using khepri::QuadraticPolynomial;
using khepri::QuarticPolynomial;
using testing::DoubleNear;
using testing::ElementsAre;

TEST(PolynomialTest, LinearPolynomial)
{
    // f(x) = 2x + 1
    const LinearPolynomial p{1, 2};

    // Sample
    EXPECT_EQ(p.sample(0), 1);
    EXPECT_EQ(p.sample(1), 3);
    EXPECT_EQ(p.sample(10), 21);

    // Solve
    constexpr double max_error = 1e-6;
    EXPECT_THAT(p.solve(6), ElementsAre(DoubleNear(2.5, max_error)));
    EXPECT_THAT(p.solve(2), ElementsAre(DoubleNear(0.5, max_error)));
    EXPECT_THAT(p.solve(0), ElementsAre(DoubleNear(-0.5, max_error)));
    EXPECT_THAT(p.solve(-100), ElementsAre(DoubleNear(-50.5, max_error)));

    // Sample first derivative (f'(x) = 2)
    const auto& d = p.derivative();
    EXPECT_EQ(d.sample(0), 2);
    EXPECT_EQ(d.sample(1), 2);
    EXPECT_EQ(d.sample(10), 2);

    // Sample second derivative (f''(x) = 0)
    const auto& dd = d.derivative();
    EXPECT_EQ(dd.sample(0), 0);
    EXPECT_EQ(dd.sample(1), 0);
    EXPECT_EQ(dd.sample(10), 0);
}

TEST(PolynomialTest, QuadraticPolynomial)
{
    // f(x) = 3x² + 2x + 1, a parabola with its minimum at (-1/3, 2/3).
    const QuadraticPolynomial p{1, 2, 3};

    // Sample
    EXPECT_EQ(p.sample(0), 1);
    EXPECT_EQ(p.sample(1), 6);
    EXPECT_EQ(p.sample(10), 321);

    // Solve
    constexpr double max_error = 1e-6;
    EXPECT_THAT(p.solve(0), ElementsAre());
    EXPECT_THAT(p.solve(0.5), ElementsAre());
    EXPECT_THAT(p.solve(2), ElementsAre(DoubleNear(-1, max_error), DoubleNear(1.0 / 3, max_error)));
    EXPECT_THAT(p.solve(100),
                ElementsAre(DoubleNear(-6.08755883, max_error), DoubleNear(5.42089217, max_error)));

    // Sample first derivative (f'(x) = 6x + 2)
    const auto& d = p.derivative();
    EXPECT_EQ(d.sample(0), 2);
    EXPECT_EQ(d.sample(1), 8);
    EXPECT_EQ(d.sample(10), 62);

    // Sample second derivative (f''(x) = 6)
    const auto& dd = d.derivative();
    EXPECT_EQ(dd.sample(0), 6);
    EXPECT_EQ(dd.sample(1), 6);
    EXPECT_EQ(dd.sample(10), 6);

    // Sample third derivative (f'''(x) = 0)
    const auto& ddd = dd.derivative();
    EXPECT_EQ(ddd.sample(0), 0);
    EXPECT_EQ(ddd.sample(1), 0);
    EXPECT_EQ(ddd.sample(10), 0);
}

TEST(PolynomialTest, CubicPolynomial)
{
    // f(x) = 4x³ + 3x² + 2x + 1, a cubic without local minima or maxima
    const CubicPolynomial p{1, 2, 3, 4};
    // Sample
    EXPECT_EQ(p.sample(0), 1);
    EXPECT_EQ(p.sample(1), 10);
    EXPECT_EQ(p.sample(10), 4321);

    // Solve. Because it has no local minima or maxima, there's only ever one solution.
    constexpr double max_error = 1e-6;
    EXPECT_THAT(p.solve(-10), ElementsAre(DoubleNear(-1.55977729, max_error)));
    EXPECT_THAT(p.solve(0), ElementsAre(DoubleNear(-0.60582959, max_error)));
    EXPECT_THAT(p.solve(1), ElementsAre(DoubleNear(0, max_error)));
    EXPECT_THAT(p.solve(10), ElementsAre(DoubleNear(1, max_error)));

    // Solve f(x) = 4x³ - 3x² + 1, with a local maximum at (0, 1) and a local minimum at (2, -3).
    const CubicPolynomial p2{1, 0, -3, 1};
    // Local minimum/maximum; two solutions
    EXPECT_THAT(p2.solve(1), ElementsAre(DoubleNear(0, max_error), DoubleNear(3, max_error)));
    EXPECT_THAT(p2.solve(-3), ElementsAre(DoubleNear(-1, max_error), DoubleNear(2, max_error)));
    // Point in between, three solutions
    EXPECT_THAT(p2.solve(0),
                ElementsAre(DoubleNear(-0.53208889, max_error), DoubleNear(0.65270364, max_error),
                            DoubleNear(2.87938524, max_error)));
    // Outside: one solution
    EXPECT_THAT(p2.solve(5), ElementsAre(DoubleNear(3.3553014, max_error)));
    EXPECT_THAT(p2.solve(-5), ElementsAre(DoubleNear(-1.19582335, max_error)));

    // Sample first derivative (f'(x) = 12x² + 6x + 2)
    const auto& d = p.derivative();
    EXPECT_EQ(d.sample(0), 2);
    EXPECT_EQ(d.sample(1), 20);
    EXPECT_EQ(d.sample(10), 1262);

    // Sample second derivative (f''(x) = 24x + 6)
    const auto& dd = d.derivative();
    EXPECT_EQ(dd.sample(0), 6);
    EXPECT_EQ(dd.sample(1), 30);
    EXPECT_EQ(dd.sample(10), 246);

    // Sample third derivative (f'''(x) = 24)
    const auto& ddd = dd.derivative();
    EXPECT_EQ(ddd.sample(0), 24);
    EXPECT_EQ(ddd.sample(1), 24);
    EXPECT_EQ(ddd.sample(10), 24);

    // Sample fourth derivative (f''''(x) = 0)
    const auto& dddd = ddd.derivative();
    EXPECT_EQ(dddd.sample(0), 0);
    EXPECT_EQ(dddd.sample(1), 0);
    EXPECT_EQ(dddd.sample(10), 0);
}

TEST(PolynomialTest, QuarticPolynomial)
{
    // f(x) = 5x⁴ + 4x³ + 3x² + 2x + 1, a quartic with minimum at approx. (-0.4370802, 0.54743896)
    const QuarticPolynomial p{1, 2, 3, 4, 5};
    // Sample
    EXPECT_EQ(p.sample(0), 1);
    EXPECT_EQ(p.sample(1), 15);
    EXPECT_EQ(p.sample(10), 54321);

    // Solve.
    constexpr double max_error = 1e-6;
    EXPECT_THAT(p.solve(-10), ElementsAre());
    EXPECT_THAT(p.solve(0), ElementsAre());
    EXPECT_THAT(p.solve(1),
                ElementsAre(DoubleNear(-0.72932314, max_error), DoubleNear(0, max_error)));
    EXPECT_THAT(p.solve(10),
                ElementsAre(DoubleNear(-1.33371806, max_error), DoubleNear(0.85234477, max_error)));

    // Solve f(x) = x⁴ = 0, which has one real solutions
    EXPECT_THAT(QuarticPolynomial({0, 0, 0, 0, 1}).solve(0), ElementsAre(DoubleNear(0, max_error)));

    // Solve f(x) = x⁴ + 4x³ - 8x² = -1, which has four real solutions
    QuarticPolynomial p2{0, 0, -8, 4, 1};
    EXPECT_THAT(p2.solve(-1),
                ElementsAre(DoubleNear(-5.45925525, max_error), DoubleNear(-0.32952020, max_error),
                            DoubleNear(0.40037871, max_error), DoubleNear(1.38839673, max_error)));

    // Solve f(x) = x⁴ + x² + 5 = 5, which has one real solution
    EXPECT_THAT(QuarticPolynomial({5, 0, 1, 0, 1}).solve(5), ElementsAre(DoubleNear(0, max_error)));

    // Sample first derivative: f'(x) = 20x³ + 12x² + 6x + 2
    const auto& d = p.derivative();
    EXPECT_EQ(d.sample(0), 2);
    EXPECT_EQ(d.sample(1), 40);
    EXPECT_EQ(d.sample(10), 21262);

    // Sample second derivative: f''(x) = 60x² + 24x + 6
    const auto& dd = d.derivative();
    EXPECT_EQ(dd.sample(0), 6);
    EXPECT_EQ(dd.sample(1), 90);
    EXPECT_EQ(dd.sample(10), 6246);

    // Sample third derivative: f'''(x) = 120x + 24
    const auto& ddd = dd.derivative();
    EXPECT_EQ(ddd.sample(0), 24);
    EXPECT_EQ(ddd.sample(1), 144);
    EXPECT_EQ(ddd.sample(10), 1224);

    // Sample fourth derivative: f''''(x) = 120
    const auto& dddd = ddd.derivative();
    EXPECT_EQ(dddd.sample(0), 120);
    EXPECT_EQ(dddd.sample(1), 120);
    EXPECT_EQ(dddd.sample(10), 120);

    // Sample fifth derivative: f'''''(x) = 0
    const auto& ddddd = dddd.derivative();
    EXPECT_EQ(ddddd.sample(0), 0);
    EXPECT_EQ(ddddd.sample(1), 0);
    EXPECT_EQ(ddddd.sample(10), 0);
}

TEST(PolynomialTest, QuinticPolynomial)
{
    using QuinticPolynomial = Polynomial<5>;

    // f(x) = 6x⁵ + 5x⁴ + 4x³ + 3x² + 2x + 1
    const QuinticPolynomial p{1, 2, 3, 4, 5, 6};

    // Sample
    EXPECT_EQ(p.sample(0), 1);
    EXPECT_EQ(p.sample(1), 21);
    EXPECT_EQ(p.sample(10), 654321);

    // We can't solve polynomials of fifth degree or higher.
}
