#include "matchers.hpp"

#include <khepri/math/ios.hpp>
#include <khepri/math/math.hpp>
#include <khepri/math/matrix.hpp>
#include <khepri/math/quaternion.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>

using khepri::ExtrinsicRotationOrder;
using khepri::IntrinsicRotationOrder;
using khepri::Quaternion;
using khepri::to_radians;
using khepri::Vector3;

//
// All tests in this file assume a right-handed system, and a right-handed rotation.
//
namespace {
void CheckCorrectRotation(const Quaternion& q, const Vector3& new_x, const Vector3& new_y,
                          const Vector3& new_z)
{
    EXPECT_THAT(Vector3(1, 0, 0) * q, IsNearVector3(new_x, 0.001));
    EXPECT_THAT(Vector3(0, 1, 0) * q, IsNearVector3(new_y, 0.001));
    EXPECT_THAT(Vector3(0, 0, 1) * q, IsNearVector3(new_z, 0.001));
}
} // namespace

TEST(QuaternionTest, OneEulerAngle_Intrinstic)
{
    // Rotate 90° around X.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), 0, 0, IntrinsicRotationOrder::xyz), {1, 0, 0},
        {0, 0, 1}, {0, -1, 0});

    // Rotate 90° around Y.
    CheckCorrectRotation(
        Quaternion::from_euler(0, to_radians(90.0), 0, IntrinsicRotationOrder::xyz), {0, 0, -1},
        {0, 1, 0}, {1, 0, 0});

    // Rotate 90° around Z.
    CheckCorrectRotation(
        Quaternion::from_euler(0, 0, to_radians(90.0), IntrinsicRotationOrder::xyz), {0, 1, 0},
        {-1, 0, 0}, {0, 0, 1});
}

TEST(QuaternionTest, TwoEulerAngles_Intrinstic)
{
    // Rotate 90° around X, then 90° around Y.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), to_radians(90.0), 0, IntrinsicRotationOrder::xyz),
        {0, 1, 0}, {0, 0, 1}, {1, 0, 0});

    // Rotate 90° around Y, then 90° around X.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), to_radians(90.0), 0, IntrinsicRotationOrder::zyx),
        {0, 0, -1}, {1, 0, 0}, {0, -1, 0});

    // Rotate 90° around X, then 90° around Z.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), 0, to_radians(90.0), IntrinsicRotationOrder::xyz),
        {0, 0, 1}, {-1, 0, 0}, {0, -1, 0});

    // Rotate 90° around Z, then 90° around X.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), 0, to_radians(90.0), IntrinsicRotationOrder::zyx),
        {0, 1, 0}, {0, 0, 1}, {1, 0, 0});

    // Rotate 90° around Y, then 90° around Z.
    CheckCorrectRotation(
        Quaternion::from_euler(0, to_radians(90.0), to_radians(90.0), IntrinsicRotationOrder::xyz),
        {0, 1, 0}, {0, 0, 1}, {1, 0, 0});

    // Rotate 90° around Z, then 90° around Y.
    CheckCorrectRotation(
        Quaternion::from_euler(0, to_radians(90.0), to_radians(90.0), IntrinsicRotationOrder::zyx),
        {0, 0, -1}, {-1, 0, 0}, {0, 1, 0});
}

TEST(QuaternionTest, ThreeEulerAngles_Intrinstic)
{
    // Rotate 90° around X, then 90° around Y, then 90° around Z.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), IntrinsicRotationOrder::xyz),
                         {0, 0, 1}, {0, -1, 0}, {1, 0, 0});

    // Rotate 90° around X, then 90° around Z, then 90° around Y.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), IntrinsicRotationOrder::xzy),
                         {0, 1, 0}, {-1, 0, 0}, {0, 0, 1});

    // Rotate 90° around Y, then 90° around X, then 90° around Z.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), IntrinsicRotationOrder::yxz),
                         {1, 0, 0}, {0, 0, 1}, {0, -1, 0});

    // Rotate 90° around Y, then 90° around Z, then 90° around X.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), IntrinsicRotationOrder::yzx),
                         {0, 1, 0}, {1, 0, 0}, {0, 0, -1});

    // Rotate 90° around Z, then 90° around X, then 90° around Y.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), IntrinsicRotationOrder::zxy),
                         {-1, 0, 0}, {0, 0, 1}, {0, 1, 0});

    // Rotate 90° around Z, then 90° around Y, then 90° around X.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), IntrinsicRotationOrder::zyx),
                         {0, 0, -1}, {0, 1, 0}, {1, 0, 0});
}

TEST(QuaternionTest, OneEulerAngle_Extrinstic)
{
    // Rotate 90° around X.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), 0, 0, ExtrinsicRotationOrder::xyz), {1, 0, 0},
        {0, 0, 1}, {0, -1, 0});

    // Rotate 90° around Y.
    CheckCorrectRotation(
        Quaternion::from_euler(0, to_radians(90.0), 0, ExtrinsicRotationOrder::xyz), {0, 0, -1},
        {0, 1, 0}, {1, 0, 0});

    // Rotate 90° around Z.
    CheckCorrectRotation(
        Quaternion::from_euler(0, 0, to_radians(90.0), ExtrinsicRotationOrder::xyz), {0, 1, 0},
        {-1, 0, 0}, {0, 0, 1});
}

TEST(QuaternionTest, TwoEulerAngles_Extrinstic)
{
    // Rotate 90° around X, then 90° around Y.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), to_radians(90.0), 0, ExtrinsicRotationOrder::xyz),
        {0, 0, -1}, {1, 0, 0}, {0, -1, 0});

    // Rotate 90° around Y, then 90° around X.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), to_radians(90.0), 0, ExtrinsicRotationOrder::zyx),
        {0, 1, 0}, {0, 0, 1}, {1, 0, 0});

    // Rotate 90° around X, then 90° around Z.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), 0, to_radians(90.0), ExtrinsicRotationOrder::xyz),
        {0, 1, 0}, {0, 0, 1}, {1, 0, 0});

    // Rotate 90° around Z, then 90° around X.
    CheckCorrectRotation(
        Quaternion::from_euler(to_radians(90.0), 0, to_radians(90.0), ExtrinsicRotationOrder::zyx),
        {0, 0, 1}, {-1, 0, 0}, {0, -1, 0});

    // Rotate 90° around Y, then 90° around Z.
    CheckCorrectRotation(
        Quaternion::from_euler(0, to_radians(90.0), to_radians(90.0), ExtrinsicRotationOrder::xyz),
        {0, 0, -1}, {-1, 0, 0}, {0, 1, 0});

    // Rotate 90° around Z, then 90° around Y.
    CheckCorrectRotation(
        Quaternion::from_euler(0, to_radians(90.0), to_radians(90.0), ExtrinsicRotationOrder::zyx),
        {0, 1, 0}, {0, 0, 1}, {1, 0, 0});
}

TEST(QuaternionTest, ThreeEulerAngles_Extrinstic)
{
    // Rotate 90° around X, then 90° around Y, then 90° around Z.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), ExtrinsicRotationOrder::xyz),
                         {0, 0, -1}, {0, 1, 0}, {1, 0, 0});

    // Rotate 90° around X, then 90° around Z, then 90° around Y.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), ExtrinsicRotationOrder::xzy),
                         {0, 1, 0}, {1, 0, 0}, {0, 0, -1});

    // Rotate 90° around Y, then 90° around X, then 90° around Z.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), ExtrinsicRotationOrder::yxz),
                         {-1, 0, 0}, {0, 0, 1}, {0, 1, 0});

    // Rotate 90° around Y, then 90° around Z, then 90° around X.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), ExtrinsicRotationOrder::yzx),
                         {0, 1, 0}, {-1, 0, 0}, {0, 0, 1});

    // Rotate 90° around Z, then 90° around X, then 90° around Y.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), ExtrinsicRotationOrder::zxy),
                         {1, 0, 0}, {0, 0, 1}, {0, -1, 0});

    // Rotate 90° around Z, then 90° around Y, then 90° around X.
    CheckCorrectRotation(Quaternion::from_euler(to_radians(90.0), to_radians(90.0),
                                                to_radians(90.0), ExtrinsicRotationOrder::zyx),
                         {0, 0, 1}, {0, -1, 0}, {1, 0, 0});
}
