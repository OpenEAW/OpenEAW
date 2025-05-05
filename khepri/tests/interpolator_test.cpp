#include <khepri/exceptions.hpp>
#include <khepri/math/interpolator.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>

using khepri::ArgumentError;
using khepri::CosineInterpolator;
using khepri::CubicInterpolator;
using khepri::Interpolator;
using khepri::LinearInterpolator;
using khepri::Point;
using khepri::StepInterpolator;

namespace {
double tangent(const Interpolator& interpolator, double x) noexcept
{
    const double dx = 0.0000001;
    const double y1 = interpolator.interpolate(x);
    const double y2 = interpolator.interpolate(x + dx);
    return (y2 - y1) / dx;
}
} // namespace

TEST(InterpolatorTest, InterpolatorWithNoPoints_ThrowsArgumentError)
{
    EXPECT_THROW(StepInterpolator({}), ArgumentError);
    EXPECT_THROW(LinearInterpolator({}), ArgumentError);
    EXPECT_THROW(CosineInterpolator({}), ArgumentError);
    EXPECT_THROW(CubicInterpolator({}), ArgumentError);
}

TEST(InterpolatorTest, InterpolatorWithUnsortedPoints_ThrowsArgumentError)
{
    EXPECT_THROW(StepInterpolator({{1, 0}, {0, 0}}), ArgumentError);
    EXPECT_THROW(LinearInterpolator({{1, 0}, {0, 0}}), ArgumentError);
    EXPECT_THROW(CosineInterpolator({{1, 0}, {0, 0}}), ArgumentError);
    EXPECT_THROW(CubicInterpolator({{1, 0}, {0, 0}}), ArgumentError);
}

TEST(InterpolatorTest, InterpolatorWithDuplicatePoints_ThrowsArgumentError)
{
    EXPECT_THROW(StepInterpolator({{0, 0}, {1, 0}, {1, 1}}), ArgumentError);
    EXPECT_THROW(LinearInterpolator({{0, 0}, {1, 0}, {1, 1}}), ArgumentError);
    EXPECT_THROW(CosineInterpolator({{0, 0}, {1, 0}, {1, 1}}), ArgumentError);
    EXPECT_THROW(CubicInterpolator({{0, 0}, {1, 0}, {1, 1}}), ArgumentError);
}

TEST(InterpolatorTest, Interpolator_SampledOutOfBounds_ClampsInput)
{
    const std::vector<Point> points{{1, 1}, {3, 2}};

    EXPECT_EQ(StepInterpolator(points).interpolate(0.0), 1);
    EXPECT_EQ(LinearInterpolator(points).interpolate(0.0), 1);
    EXPECT_EQ(CosineInterpolator(points).interpolate(0.0), 1);
    EXPECT_EQ(CubicInterpolator(points).interpolate(0.0), 1);

    EXPECT_EQ(StepInterpolator(points).interpolate(4.0), 2);
    EXPECT_EQ(LinearInterpolator(points).interpolate(4.0), 2);
    EXPECT_EQ(CosineInterpolator(points).interpolate(4.0), 2);
    EXPECT_EQ(CubicInterpolator(points).interpolate(4.0), 2);
}

TEST(InterpolatorTest, StepInterpolator_InterpolatesInSteps)
{
    StepInterpolator interpolator({{0, 5}, {1.5, 3}, {3, 10}});

    // It must go through the points
    EXPECT_EQ(interpolator.interpolate(0), 5);
    EXPECT_EQ(interpolator.interpolate(1.5), 3);
    EXPECT_EQ(interpolator.interpolate(3), 10);

    // In-between it steps
    EXPECT_EQ(interpolator.interpolate(1.4), 5);
    EXPECT_EQ(interpolator.interpolate(2.9), 3);
}

TEST(InterpolatorTest, LinearInterpolatorFromOnePoint_CreatesHorizontalLine)
{
    const auto ci = LinearInterpolator({{9, 42}});
    EXPECT_EQ(ci.interpolate(0), 42);
    EXPECT_EQ(ci.interpolate(9), 42);
    EXPECT_EQ(ci.interpolate(1e10), 42);
}

TEST(InterpolatorTest, LinearInterpolator_InterpolatesLinearly)
{
    LinearInterpolator interpolator({{0, 5}, {1.5, 3}, {3, 11}});

    // It must go through the points
    EXPECT_EQ(interpolator.interpolate(0), 5);
    EXPECT_EQ(interpolator.interpolate(1.5), 3);
    EXPECT_EQ(interpolator.interpolate(3), 11);

    // In-between it goes linearly
    EXPECT_EQ(interpolator.interpolate(0.75), 4);
    EXPECT_EQ(interpolator.interpolate(2.25), 7);
    EXPECT_EQ(interpolator.interpolate(2.625), 9);
}

TEST(InterpolatorTest, CosineInterpolatorFromOnePoint_CreatesHorizontalLine)
{
    const auto ci = CosineInterpolator({{9, 42}});
    EXPECT_EQ(ci.interpolate(0), 42);
    EXPECT_EQ(ci.interpolate(9), 42);
    EXPECT_EQ(ci.interpolate(1e10), 42);
}

TEST(InterpolatorTest, CosineInterpolator_InterpolatesSmoothly)
{
    CosineInterpolator interpolator({{0, 5}, {1.5, 3}, {3, 11}});

    // It must go through the points
    EXPECT_EQ(interpolator.interpolate(0), 5);
    EXPECT_EQ(interpolator.interpolate(1.5), 3);
    EXPECT_EQ(interpolator.interpolate(3), 11);

    // Half-way between points it's half-way between values
    EXPECT_EQ(interpolator.interpolate(0.75), 4);
    EXPECT_EQ(interpolator.interpolate(2.25), 7);

    // Close to a point the tangent should be near 0
    EXPECT_NEAR(tangent(interpolator, 0.000001), 0.0, 0.0001);
    EXPECT_NEAR(tangent(interpolator, 1.499999), 0.0, 0.0001);
    EXPECT_NEAR(tangent(interpolator, 1.500001), 0.0, 0.0001);
    EXPECT_NEAR(tangent(interpolator, 2.999999), 0.0, 0.0001);
}

TEST(InterpolatorTest, CubicInterpolatorFromOnePoint_CreatesHorizontalLine)
{
    const auto ci = CubicInterpolator({{9, 42}});
    EXPECT_EQ(ci.interpolate(0), 42);
    EXPECT_EQ(ci.interpolate(9), 42);
    EXPECT_EQ(ci.interpolate(1e10), 42);
}

TEST(InterpolatorTest, CubicInterpolatorFromValidPoints_InterpolatesSmoothly)
{
    const std::vector<Point> points{{-1.5, -1.2}, {-0.2, 0}, {1, 0.5},
                                    {1.5, 1.2},   {15, 2},   {20, 1}};
    const CubicInterpolator  interpolator(points);

    for (std::size_t i = 0; i < points.size(); ++i) {
        constexpr auto MAX_ERROR = 0.00001;

        // Interpolator should go through the point
        EXPECT_NEAR(interpolator.interpolate(points[i].x), points[i].y, MAX_ERROR)
            << "for point " << i;
    }

    for (std::size_t i = 1; i < points.size() - 1; ++i) {
        constexpr auto MAX_ERROR = 0.00001;

        // Tangent on both sides of a point should be equal
        const double tangent1 = tangent(interpolator, points[i].x - 0.000001);
        const double tangent2 = tangent(interpolator, points[i].x + 0.000001);
        EXPECT_NEAR(tangent1, tangent2, MAX_ERROR) << "for point " << i;
    }
}
