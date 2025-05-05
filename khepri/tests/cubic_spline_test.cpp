#include "printers.hpp"

#include <khepri/exceptions.hpp>
#include <khepri/math/spline.hpp>
#include <khepri/utility/string.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>

using khepri::CubicSpline;
using khepri::Vector3;
using testing::TestWithParam;
using testing::Values;

namespace khepri {
std::ostream& operator<<(std::ostream& os, const CubicSpline& c)
{
    return os << "spline through [" << khepri::join(c.points(), ", ") << "]";
}

[[nodiscard]] bool near(const Vector3& lhs, const Vector3& rhs, float abs_error)
{
    const auto& near = [&](float lhs, float rhs) { return std::abs(lhs - rhs) <= abs_error; };

    return near(lhs.x, rhs.x) && near(lhs.y, rhs.y) && near(lhs.z, rhs.z);
}

} // namespace khepri

namespace {

auto Near(const Vector3& rhs, float abs_error = 0.001f)
{
    class Vector3NearMatcher
    {
    public:
        using is_gtest_matcher = void;

        explicit Vector3NearMatcher(const Vector3& rhs, float abs_error)
            : m_rhs(rhs), m_abs_error(abs_error)
        {}

        bool MatchAndExplain(const Vector3& lhs, std::ostream*) const
        {
            return near(lhs, m_rhs, m_abs_error);
        }

        void DescribeTo(std::ostream* os) const
        {
            *os << "is near " << m_rhs;
        }

        void DescribeNegationTo(std::ostream* os) const
        {
            *os << "isn't near " << m_rhs;
        }

    private:
        const Vector3 m_rhs;
        float         m_abs_error;
    };

    return Vector3NearMatcher(rhs, abs_error);
}

// Performs basic checks on the spline such as interpolation of the configured points and C2
// continuity.
auto IsValidSpline()
{
    static constexpr auto MAX_ERROR = 0.0001f;

    class ValidSplineMatcher
    {
    public:
        using is_gtest_matcher = void;

        bool MatchAndExplain(const CubicSpline& spline, std::ostream* os) const
        {
            if (!IsSplineInterpolating(spline)) {
                if (os != nullptr) {
                    *os << "spline is not interpolating";
                }
                return false;
            }

            if (!IsSplineContinuousInPosition(spline)) {
                if (os != nullptr) {
                    *os << "spline is not continuous in position";
                }
                return false;
            }

            if (!IsSplineContinuousInTangent(spline)) {
                if (os != nullptr) {
                    *os << "spline is not continuous in tangent";
                }
                return false;
            }

            if (!IsSplineContinuousInCurvature(spline)) {
                if (os != nullptr) {
                    *os << "spline is not continuous in curvature";
                }
                return false;
            }
            return true;
        }

        void DescribeTo(std::ostream* os) const
        {
            *os << "is a valid spline";
        }

        void DescribeNegationTo(std::ostream* os) const
        {
            *os << "is not a valid spline";
        }

    private:
        static bool IsSplineInterpolating(const CubicSpline& spline) noexcept
        {
            const auto& points = spline.points();
            for (std::size_t i = 0; i < points.size(); ++i) {
                const float t     = spline.length_at(i) / spline.length();
                const auto  value = spline.sample(t);
                if (!near(value, points[i], MAX_ERROR)) {
                    return false;
                }
            }
            return true;
        }

        static bool IsSplineContinuousInPosition(const CubicSpline& spline) noexcept
        {
            // Check both sides of the points to see if the positions are really close to each other
            constexpr auto offset = std::numeric_limits<float>::epsilon();
            const auto&    points = spline.points();
            for (std::size_t i = 1; i < points.size() - 1; ++i) {
                const auto t = spline.length_at(i) / spline.length();
                ;

                const auto pos_left  = spline.sample(t - offset);
                const auto pos_right = spline.sample(t + offset);
                if (!near(pos_left, pos_right, MAX_ERROR)) {
                    return false;
                }
            }
            return true;
        }

        static bool IsSplineContinuousInTangent(const CubicSpline& spline) noexcept
        {
            // Check both sides of the points to see if the tangents are really close to each other
            constexpr auto offset = std::numeric_limits<float>::epsilon();
            const auto&    points = spline.points();
            for (std::size_t i = 1; i < points.size() - 1; ++i) {
                const auto t = spline.length_at(i) / spline.length();
                ;

                // Note: technically we should divide by @a offset to get the tangent, but since
                // that's the same on both side of the comparison, we can ignore it.
                const auto tangent_left  = spline.sample(t) - spline.sample(t - offset);
                const auto tangent_right = spline.sample(t + offset) - spline.sample(t);
                if (!near(tangent_left, tangent_right, MAX_ERROR)) {
                    return false;
                }
            }
            return true;
        }

        static bool IsSplineContinuousInCurvature(const CubicSpline& spline) noexcept
        {
            // Check both sides of the points to see if the curvatures are really close to each
            // other
            constexpr auto offset = 0.0001f;
            const auto&    points = spline.points();
            for (std::size_t i = 1; i < points.size() - 1; ++i) {
                const auto t = static_cast<float>(i) / (points.size() - 1);

                // Note: technically we should divide by @a offset to get the tangents, but since
                // that's the same on both side of the comparison, we can ignore it. Same for
                // curvature
                const auto tangent_left_1  = spline.sample(t) - spline.sample(t - offset);
                const auto tangent_right_1 = spline.sample(t + offset) - spline.sample(t);

                const auto tangent_left_2 =
                    spline.sample(t - offset) - spline.sample(t - 2 * offset);
                const auto tangent_right_2 =
                    spline.sample(t + 2 * offset) - spline.sample(t + offset);

                const auto curvature_left  = tangent_left_1 - tangent_left_2;
                const auto curvature_right = tangent_right_2 - tangent_right_1;

                if (!near(curvature_left, curvature_right, MAX_ERROR)) {
                    return false;
                }
            }
            return true;
        }
    };

    return ValidSplineMatcher{};
}

} // namespace

TEST(CubicSplineTest, SplineWithZeroPoints_ThrowsArgumentError)
{
    EXPECT_THROW(CubicSpline{{}}, khepri::ArgumentError);
}

TEST(CubicSplineTest, SplineWithOnePoint_ThrowsArgumentError)
{
    static constexpr Vector3 points[] = {{0, 0, 0}};
    EXPECT_THROW(CubicSpline{points}, khepri::ArgumentError);
}

TEST(CubicSplineTest, LineSpline_HasCorrectLength)
{
    static constexpr Vector3 points[] = {{0, 0, 0}, {1, 1, 1}};
    EXPECT_NEAR(CubicSpline{points}.length(), std::sqrt(3.0), 0.00000001f);
}

class ValidCubicSplineTest : public testing::TestWithParam<std::vector<Vector3>>
{};

// Test that the spline is valid (interpolating & continuous)
TEST_P(ValidCubicSplineTest, SplineIsValid)
{
    CubicSpline spline(GetParam());
    EXPECT_THAT(spline, IsValidSpline());
}

// Test that when sampling at a regular interval along the spline, the distance between returned
// points is roughly the same.
TEST_P(ValidCubicSplineTest, SplinePointsAreEquidistant)
{
    static constexpr auto MAX_ERROR = 0.0001f;

    CubicSpline spline(GetParam());

    std::vector<Vector3> samples(201);
    const auto           sample_length = spline.length() / (samples.size() - 1);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        auto t     = static_cast<float>(i) / (samples.size() - 1);
        samples[i] = spline.sample(t);
    }

    const float distance = khepri::distance(samples[1], samples[0]);
    for (std::size_t i = 1; i < samples.size() - 1; ++i) {
        EXPECT_NEAR(distance, sample_length, MAX_ERROR) << " for point " << i;
    }
}

INSTANTIATE_TEST_CASE_P(ValidCubicSplineTest, ValidCubicSplineTest,
                        Values(std::vector<Vector3>{{{0, 0, 0}, {1, 1, 1}}},
                               std::vector<Vector3>{{{0, 0, 0}, {1, 1, 0}, {2, 0, 0}}},
                               std::vector<Vector3>{{{0, 1, 0}, {2, 2, 0}, {5, 0, 0}, {8, 0, 0}}},
                               std::vector<Vector3>{
                                   {{5, -7, 0}, {2, 0, 0}, {1, 3, 0}, {2, 2, 0}, {5, -3, 0}}},
                               std::vector<Vector3>{{{-1.5, -1.2, 0},
                                                     {-0.2, 0, 0},
                                                     {1, 0.5, 0},
                                                     {5, 1, 0},
                                                     {10, 1.2, 0},
                                                     {15, 2, 0},
                                                     {20, 1, 0}}}));
