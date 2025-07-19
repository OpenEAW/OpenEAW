#include "matchers.hpp"

#include <khepri/math/ios.hpp>
#include <khepri/math/math.hpp>
#include <khepri/math/matrix.hpp>
#include <khepri/math/quaternion.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>

using khepri::Matrix;
using khepri::Quaternion;
using khepri::Vector3;

TEST(MatrixTest, RotationMatrixHandedness)
{
    {
        // Rotate 90 degrees around the X axis.
        // This assumes a right-handed system, and a right-handed rotation.
        const auto rot = Matrix::create_rotation(
            Quaternion::from_axis_angle({1, 0, 0}, khepri::to_radians(90.0)));
        EXPECT_THAT(Vector3(1, 0, 0) * rot, IsNearVector3(Vector3{1, 0, 0}, 0.001));
        EXPECT_THAT(Vector3(0, 1, 0) * rot, IsNearVector3(Vector3{0, 0, 1}, 0.001));
        EXPECT_THAT(Vector3(0, 0, 1) * rot, IsNearVector3(Vector3{0, -1, 0}, 0.001));
    }

    {
        // Rotate 90 degrees around the Y axis.
        // This assumes a right-handed system, and a right-handed rotation.
        const auto rot = Matrix::create_rotation(
            Quaternion::from_axis_angle({0, 1, 0}, khepri::to_radians(90.0)));
        EXPECT_THAT(Vector3(1, 0, 0) * rot, IsNearVector3(Vector3{0, 0, -1}, 0.001));
        EXPECT_THAT(Vector3(0, 1, 0) * rot, IsNearVector3(Vector3{0, 1, 0}, 0.001));
        EXPECT_THAT(Vector3(0, 0, 1) * rot, IsNearVector3(Vector3{1, 0, 0}, 0.001));
    }

    {
        // Rotate 90 degrees around the Z axis.
        // This assumes a right-handed system, and a right-handed rotation.
        const auto rot = Matrix::create_rotation(
            Quaternion::from_axis_angle({0, 0, 1}, khepri::to_radians(90.0)));
        EXPECT_THAT(Vector3(1, 0, 0) * rot, IsNearVector3(Vector3{0, 1, 0}, 0.001));
        EXPECT_THAT(Vector3(0, 1, 0) * rot, IsNearVector3(Vector3{-1, 0, 0}, 0.001));
        EXPECT_THAT(Vector3(0, 0, 1) * rot, IsNearVector3(Vector3{0, 0, 1}, 0.001));
    }
}
