#include "printers.hpp"

#include <khepri/math/polynomial.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using khepri::CubicPolynomial;
using khepri::LinearPolynomial;
using khepri::QuadraticPolynomial;

TEST(PolynomialTest, LinearPolynomial)
{
    const LinearPolynomial p{1, 2};
    EXPECT_EQ(p.sample(0), 1);
    EXPECT_EQ(p.sample(1), 3);
    EXPECT_EQ(p.sample(10), 21);

    const auto& d = p.derivative();
    EXPECT_EQ(d.sample(0), 2);
    EXPECT_EQ(d.sample(1), 2);
    EXPECT_EQ(d.sample(10), 2);

    const auto& dd = d.derivative();
    EXPECT_EQ(dd.sample(0), 0);
    EXPECT_EQ(dd.sample(1), 0);
    EXPECT_EQ(dd.sample(10), 0);
}

TEST(PolynomialTest, QuadraticPolynomial)
{
    const QuadraticPolynomial p{1, 2, 3};
    EXPECT_EQ(p.sample(0), 1);
    EXPECT_EQ(p.sample(1), 6);
    EXPECT_EQ(p.sample(10), 321);

    const auto& d = p.derivative();
    EXPECT_EQ(d.sample(0), 2);
    EXPECT_EQ(d.sample(1), 8);
    EXPECT_EQ(d.sample(10), 62);

    const auto& dd = d.derivative();
    EXPECT_EQ(dd.sample(0), 6);
    EXPECT_EQ(dd.sample(1), 6);
    EXPECT_EQ(dd.sample(10), 6);

    const auto& ddd = dd.derivative();
    EXPECT_EQ(ddd.sample(0), 0);
    EXPECT_EQ(ddd.sample(1), 0);
    EXPECT_EQ(ddd.sample(10), 0);
}

TEST(PolynomialTest, CubicPolynomial)
{
    const CubicPolynomial p{1, 2, 3, 4};
    EXPECT_EQ(p.sample(0), 1);
    EXPECT_EQ(p.sample(1), 10);
    EXPECT_EQ(p.sample(10), 4321);

    const auto& d = p.derivative();
    EXPECT_EQ(d.sample(0), 2);
    EXPECT_EQ(d.sample(1), 20);
    EXPECT_EQ(d.sample(10), 1262);

    const auto& dd = d.derivative();
    EXPECT_EQ(dd.sample(0), 6);
    EXPECT_EQ(dd.sample(1), 30);
    EXPECT_EQ(dd.sample(10), 246);

    const auto& ddd = dd.derivative();
    EXPECT_EQ(ddd.sample(0), 24);
    EXPECT_EQ(ddd.sample(1), 24);
    EXPECT_EQ(ddd.sample(10), 24);

    const auto& dddd = ddd.derivative();
    EXPECT_EQ(dddd.sample(0), 0);
    EXPECT_EQ(dddd.sample(1), 0);
    EXPECT_EQ(dddd.sample(10), 0);
}