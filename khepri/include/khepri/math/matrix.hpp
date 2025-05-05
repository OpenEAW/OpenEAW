#pragma once

#include "vector3.hpp"
#include "vector4.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <type_traits>

namespace khepri {

template <typename ComponentT>
class BasicQuaternion;

using Quaternion = BasicQuaternion<double>;

/**
 * \brief 4x4 matrix
 *
 * \note the matrix is stored in column-major order.
 */
template <typename ComponentT>
class BasicMatrix final
{
    std::array<BasicVector4<ComponentT>, 4> m_cols{};

public:
    /// The type of the matrix's components
    using ComponentType = ComponentT;

    /// Constructs an uninitialized matrix
    BasicMatrix() noexcept = default;

    /// Constructs the matrix from 16 floats, row-major order
    BasicMatrix(ComponentType m11, ComponentType m12, ComponentType m13, ComponentType m14,
                ComponentType m21, ComponentType m22, ComponentType m23, ComponentType m24,
                ComponentType m31, ComponentType m32, ComponentType m33, ComponentType m34,
                ComponentType m41, ComponentType m42, ComponentType m43, ComponentType m44) noexcept
    {
        m_cols[0] = {m11, m21, m31, m41};
        m_cols[1] = {m12, m22, m32, m42};
        m_cols[2] = {m13, m23, m33, m43};
        m_cols[3] = {m14, m24, m34, m44};
    }

    /// Constructs the matrix from 4 column vectors
    BasicMatrix(const BasicVector4<ComponentType>& v1, const BasicVector4<ComponentType>& v2,
                const BasicVector4<ComponentType>& v3,
                const BasicVector4<ComponentType>& v4) noexcept
    {
        m_cols[0] = v1;
        m_cols[1] = v2;
        m_cols[2] = v3;
        m_cols[3] = v4;
    }

    /// Post-multiplies the matrix with \a mat
    BasicMatrix& operator*=(const BasicMatrix& mat) noexcept
    {
        return *this = *this * mat;
    }

    /// Scales all elements of the matrix
    BasicMatrix& operator*=(ComponentType s) noexcept
    {
        m_cols[0] *= s;
        m_cols[1] *= s;
        m_cols[2] *= s;
        m_cols[3] *= s;
        return *this;
    }

    /// Scales all elements of the matrix
    BasicMatrix& operator/=(ComponentType s) noexcept
    {
        return *this *= (1.0 / s);
    }

    /// Returns an element of the matrix
    /// \throws std::out_of_range if \a row or \a col is not between 0 and 3, inclusive.
    constexpr const auto& operator()(std::size_t row, std::size_t col) const
    {
        return m_cols.at(col)[row];
    }

    /// Returns an element of the matrix
    /// \throws std::out_of_range if \a row or \a col is not between 0 and 3, inclusive.
    constexpr auto& operator()(std::size_t row, std::size_t col)
    {
        return m_cols.at(col)[row];
    }

    /// Returns a column of the matrix
    /// \throws std::out_of_range if \a col is not between 0 and 3, inclusive.
    constexpr BasicVector4<ComponentType> col(std::size_t col) const noexcept
    {
        return m_cols.at(col);
    }

    /// Returns a row of the matrix
    /// \throws std::out_of_range if \a row is not between 0 and 3, inclusive.
    constexpr BasicVector4<ComponentType> row(std::size_t row) const noexcept
    {
        return {m_cols[0][row], m_cols[1][row], m_cols[2][row], m_cols[3][row]};
    }

    /// Transform (post-multiply) \a v with this matrix, assuming 1.0 as w component
    template <typename U>
    [[nodiscard]] auto transform_coord(const BasicVector3<U>& v) const noexcept
    {
        const auto a = BasicVector4<U>(v, 1) * *this;
        return BasicVector3<std::common_type_t<ComponentType, U>>{a.x / a.w, a.y / a.w, a.z / a.w};
    }

    /// Transposes the matrix
    void transpose() noexcept
    {
        using std::swap;
        for (std::size_t i = 1; i < 4; i++) {
            for (std::size_t j = 0; j < i; j++) {
                swap((*this)(i, j), (*this)(j, i));
            }
        }
    }

    /// Returns the inverse matrix.
    /// \note is undefined behavior if the matrix is not invertible.
    [[nodiscard]] BasicMatrix inverse() const noexcept
    {
        BasicMatrix        dst;
        const BasicMatrix& src = *this;

        dst(0, 0) = src(1, 1) * (src(2, 2) * src(3, 3) - src(2, 3) * src(3, 2)) -
                    src(2, 1) * (src(1, 2) * src(3, 3) - src(1, 3) * src(3, 2)) +
                    src(3, 1) * (src(1, 2) * src(2, 3) - src(1, 3) * src(2, 2));

        dst(1, 0) = src(2, 0) * (src(1, 2) * src(3, 3) - src(1, 3) * src(3, 2)) -
                    src(1, 0) * (src(2, 2) * src(3, 3) - src(2, 3) * src(3, 2)) -
                    src(3, 0) * (src(1, 2) * src(2, 3) - src(1, 3) * src(2, 2));

        dst(2, 0) = src(1, 0) * (src(2, 1) * src(3, 3) - src(2, 3) * src(3, 1)) -
                    src(2, 0) * (src(1, 1) * src(3, 3) - src(1, 3) * src(3, 1)) +
                    src(3, 0) * (src(1, 1) * src(2, 3) - src(1, 3) * src(2, 1));

        dst(3, 0) = src(2, 0) * (src(1, 1) * src(3, 2) - src(1, 2) * src(3, 1)) -
                    src(1, 0) * (src(2, 1) * src(3, 2) - src(2, 2) * src(3, 1)) -
                    src(3, 0) * (src(1, 1) * src(2, 2) - src(1, 2) * src(2, 1));

        dst(0, 1) = src(2, 1) * (src(0, 2) * src(3, 3) - src(0, 3) * src(3, 2)) -
                    src(0, 1) * (src(2, 2) * src(3, 3) - src(2, 3) * src(3, 2)) -
                    src(3, 1) * (src(0, 2) * src(2, 3) - src(0, 3) * src(2, 2));

        dst(1, 1) = src(0, 0) * (src(2, 2) * src(3, 3) - src(2, 3) * src(3, 2)) -
                    src(2, 0) * (src(0, 2) * src(3, 3) - src(0, 3) * src(3, 2)) +
                    src(3, 0) * (src(0, 2) * src(2, 3) - src(0, 3) * src(2, 2));

        dst(2, 1) = src(2, 0) * (src(0, 1) * src(3, 3) - src(0, 3) * src(3, 1)) -
                    src(0, 0) * (src(2, 1) * src(3, 3) - src(2, 3) * src(3, 1)) -
                    src(3, 0) * (src(0, 1) * src(2, 3) - src(0, 3) * src(2, 1));

        dst(3, 1) = src(0, 0) * (src(2, 1) * src(3, 2) - src(2, 2) * src(3, 1)) -
                    src(2, 0) * (src(0, 1) * src(3, 2) - src(0, 2) * src(3, 1)) +
                    src(3, 0) * (src(0, 1) * src(2, 2) - src(0, 2) * src(2, 1));

        dst(0, 2) = src(0, 1) * (src(1, 2) * src(3, 3) - src(1, 3) * src(3, 2)) -
                    src(1, 1) * (src(0, 2) * src(3, 3) - src(0, 3) * src(3, 2)) +
                    src(3, 1) * (src(0, 2) * src(1, 3) - src(0, 3) * src(1, 2));

        dst(1, 2) = src(1, 0) * (src(0, 2) * src(3, 3) - src(0, 3) * src(3, 2)) -
                    src(0, 0) * (src(1, 2) * src(3, 3) - src(1, 3) * src(3, 2)) -
                    src(3, 0) * (src(0, 2) * src(1, 3) - src(0, 3) * src(1, 2));

        dst(2, 2) = src(0, 0) * (src(1, 1) * src(3, 3) - src(1, 3) * src(3, 1)) -
                    src(1, 0) * (src(0, 1) * src(3, 3) - src(0, 3) * src(3, 1)) +
                    src(3, 0) * (src(0, 1) * src(1, 3) - src(0, 3) * src(1, 1));

        dst(3, 2) = src(1, 0) * (src(0, 1) * src(3, 2) - src(0, 2) * src(3, 1)) -
                    src(0, 0) * (src(1, 1) * src(3, 2) - src(1, 2) * src(3, 1)) -
                    src(3, 0) * (src(0, 1) * src(1, 2) - src(0, 2) * src(1, 1));

        dst(0, 3) = src(1, 1) * (src(0, 2) * src(2, 3) - src(0, 3) * src(2, 2)) -
                    src(0, 1) * (src(1, 2) * src(2, 3) - src(1, 3) * src(2, 2)) -
                    src(2, 1) * (src(0, 2) * src(1, 3) - src(0, 3) * src(1, 2));

        dst(1, 3) = src(0, 0) * (src(1, 2) * src(2, 3) - src(1, 3) * src(2, 2)) -
                    src(1, 0) * (src(0, 2) * src(2, 3) - src(0, 3) * src(2, 2)) +
                    src(2, 0) * (src(0, 2) * src(1, 3) - src(0, 3) * src(1, 2));

        dst(2, 3) = src(1, 0) * (src(0, 1) * src(2, 3) - src(0, 3) * src(2, 1)) -
                    src(0, 0) * (src(1, 1) * src(2, 3) - src(1, 3) * src(2, 1)) -
                    src(2, 0) * (src(0, 1) * src(1, 3) - src(0, 3) * src(1, 1));

        dst(3, 3) = src(0, 0) * (src(1, 1) * src(2, 2) - src(1, 2) * src(2, 1)) -
                    src(1, 0) * (src(0, 1) * src(2, 2) - src(0, 2) * src(2, 1)) +
                    src(2, 0) * (src(0, 1) * src(1, 2) - src(0, 2) * src(1, 1));

        // Calculate determinant.
        // If zero, the matrix is uninvertible.
        const auto det = (src(0, 0) * dst(0, 0) + src(0, 1) * dst(1, 0) + src(0, 2) * dst(2, 0) +
                          src(0, 3) * dst(3, 0));
        assert(det != 0);

        // Divide matrix by determinant
        return dst / det;
    }

    /// Returns the translation part (4th row) of the matrix
    [[nodiscard]] auto get_translation() const noexcept
    {
        return BasicVector3<ComponentType>(m_cols[0].w, m_cols[1].w, m_cols[2].w);
    }

    /// Sets the translation components of this matrix
    void set_translation(const BasicVector3<ComponentType>& v) noexcept
    {
        m_cols[0].w = v.x;
        m_cols[1].w = v.y;
        m_cols[2].w = v.z;
    }

    /// Returns the rotation and scale matrix (the top-left 3x3 submatrix)
    [[nodiscard]] BasicMatrix get_rotation_scale() const noexcept;

    /// Constructs a matrix from Scale, Rotation and Translation
    static BasicMatrix create_srt(const BasicVector3<ComponentType>&    scale,
                                  const BasicQuaternion<ComponentType>& rotation,
                                  const BasicVector3<ComponentType>&    translation) noexcept
    {
        assert(scale.x > 0);
        assert(scale.y > 0);
        assert(scale.z > 0);

        auto m = BasicMatrix::create_rotation(rotation);
        m.set_translation(translation);
        for (std::size_t i = 0; i < 3; ++i) {
            m(i, 0) *= scale[i];
            m(i, 1) *= scale[i];
            m(i, 2) *= scale[i];
        }
        return m;
    }

    /// Constructs a rotation transformation from the specified quaternion
    static BasicMatrix create_rotation(const BasicQuaternion<ComponentType>& q) noexcept
    {
        return {1 - 2 * (q.y * q.y + q.z * q.z),
                2 * (q.x * q.y + q.w * q.z),
                2 * (q.x * q.z - q.w * q.y),
                0,
                2 * (q.x * q.y - q.w * q.z),
                1 - 2 * (q.x * q.x + q.z * q.z),
                2 * (q.y * q.z + q.w * q.x),
                0,
                2 * (q.x * q.z + q.w * q.y),
                2 * (q.y * q.z - q.w * q.x),
                1 - 2 * (q.x * q.x + q.y * q.y),
                0,
                0,
                0,
                0,
                1};
    }

    /// Constructs a scale transformation from a scale factor
    static BasicMatrix create_scaling(ComponentType scale) noexcept
    {
        return {scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1};
    }

    /// Constructs a scale transformation from a scale vector
    static BasicMatrix create_scaling(const BasicVector3<ComponentType>& scale) noexcept
    {
        return {scale.x, 0, 0, 0, 0, scale.y, 0, 0, 0, 0, scale.z, 0, 0, 0, 0, 1};
    }

    /// Constructs a translation transformation from a translation vector
    static BasicMatrix create_translation(const BasicVector3<ComponentType>& translation) noexcept
    {
        return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, translation.x, translation.y, translation.z, 1};
    }

    /**
     * \brief Creates a right-handed camera-style transformation matrix
     *
     * \param[in] eye the world-space position of the camera
     * \param[in] at  the world-space target position the camera is looking at
     * \param[in] up  the world-space vector which will point up on the camera image
     */
    template <typename U>
    static BasicMatrix create_look_at_view(const BasicVector3<U>& eye, const BasicVector3<U>& at,
                                           const BasicVector3<U>& up) noexcept
    {
        const auto zaxis = normalize(eye - at);
        const auto xaxis = normalize(cross(up, zaxis));
        const auto yaxis = cross(zaxis, xaxis);

        return {
            xaxis.x, yaxis.x, zaxis.x, 0, xaxis.y,          yaxis.y,          zaxis.y,          0,
            xaxis.z, yaxis.z, zaxis.z, 0, -dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1};
    }

    /**
     * \brief Creates a right-handed perspective projection matrix
     *
     * \param[in] fovy   the vertical Field-of-View angle, in radians
     * \param[in] aspect the aspect ration (width / height) of the viewport
     * \param[in] z_near the distance from the origin to the near clip plane
     * \param[in] z_far  the distance from the origin to the far clip plane
     */

    /// Creates a right-handed perspective projection matrix
    static auto create_perspective_projection(ComponentType fovy, ComponentType aspect,
                                              ComponentType z_near, ComponentType z_far) noexcept
    {
        const auto y_scale = 1 / std::tan(fovy / 2);
        const auto x_scale = y_scale / aspect;
        const auto z_scale = z_far / (z_near - z_far);

        return BasicMatrix{x_scale, 0,       0,  0, 0, y_scale,          0, 0, 0,
                           0,       z_scale, -1, 0, 0, z_near * z_scale, 0};
    }

    /**
     * \brief Create a orthographic projection object
     *
     * \param[in] width  width of the camera image, in world-space units
     * \param[in] aspect the aspect ration (width / height) of the viewport
     * \param[in] z_near the distance from the origin to the near clip plane
     * \param[in] z_far  the distance from the origin to the far clip plane
     */
    static BasicMatrix create_orthographic_projection(ComponentType width, ComponentType aspect,
                                                      ComponentType z_near,
                                                      ComponentType z_far) noexcept
    {
        const auto height  = width / aspect;
        const auto z_scale = 1 / (z_near - z_far);
        return {2 / width, 0,       0, 0, 0, 2 / height,       0, 0, 0,
                0,         z_scale, 0, 0, 0, z_near * z_scale, 1};
    }

    /// Identity matrix
    static const BasicMatrix IDENTITY;
};

template <typename T>
const BasicMatrix<T> BasicMatrix<T>::IDENTITY(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

/// Matrix of doubles
using Matrix = BasicMatrix<double>;

/// Matrix of floats
using Matrixf = BasicMatrix<float>;

/// Transforms (Post-multiplies) a vector with a matrix
template <typename T, typename U>
auto operator*(const BasicVector4<T>& v, const BasicMatrix<U>& m) noexcept
{
    return BasicVector4<std::common_type_t<T, U>>{dot(v, m.col(0)), dot(v, m.col(1)),
                                                  dot(v, m.col(2)), dot(v, m.col(3))};
}

// Transforms (Post-multiplies) a vector with a matrix.
// Same as: Vector3(Vector4(v, 0) * m);
template <typename T>
BasicVector3<T> operator*(const BasicVector3<T>& v, const BasicMatrix<T>& m) noexcept
{
    return {v.x * m(0, 0) + v.y * m(1, 0) + v.z * m(2, 0),
            v.x * m(0, 1) + v.y * m(1, 1) + v.z * m(2, 1),
            v.x * m(0, 2) + v.y * m(1, 2) + v.z * m(2, 2)};
}

/// Post-multiplies two matrices
template <typename T>
BasicMatrix<T> operator*(const BasicMatrix<T>& m1, const BasicMatrix<T>& m2) noexcept
{
    BasicMatrix<T> m;
    for (std::size_t i = 0; i < 4; ++i)
        for (std::size_t j = 0; j < 4; ++j) {
            m(i, j) = dot(m1.row(i), m2.col(j));
        }
    return m;
}

/// Scales all elements of the matrix
template <typename T>
BasicMatrix<T> operator*(const BasicMatrix<T>& m, float s) noexcept
{
    return {m(0, 0) * s, m(0, 1) * s, m(0, 2) * s, m(0, 3) * s, m(1, 0) * s, m(1, 1) * s,
            m(1, 2) * s, m(1, 3) * s, m(2, 0) * s, m(2, 1) * s, m(2, 2) * s, m(2, 3) * s,
            m(3, 0) * s, m(3, 1) * s, m(3, 2) * s, m(3, 3) * s};
}

/// Scales all elements of the matrix
template <typename T>
auto operator*(T s, const BasicMatrix<T>& m) noexcept
{
    return m * s;
}

/// Scales all elements of the matrix
template <typename T>
auto operator/(const BasicMatrix<T>& m, T s) noexcept
{
    return m * (1.0 / s);
}

/// Returns the inverse matrix
/// \note is undefined behavior if \a m is not invertible.
template <typename T>
auto inverse(const BasicMatrix<T>& m) noexcept
{
    return m.inverse();
}

/// Transposes the matrix
template <typename T>
BasicMatrix<T> transpose(const BasicMatrix<T>& m) noexcept
{
    Matrix x;
    for (std::size_t i = 1; i < 4; i++) {
        for (std::size_t j = 0; j < i; j++) {
            x(i, j) = m(j, i);
            x(j, i) = m(i, j);
        }
    }
    return x;
}

} // namespace khepri
