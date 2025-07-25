#pragma once

#include <khepri/utility/type_traits.hpp>

#include <cmath>
#include <stdexcept>

namespace khepri {

template <typename ComponentT>
class BasicVector3;

template <typename ComponentT>
class BasicVector4;

template <typename ComponentT>
class BasicQuaternion;

/**
 * Describes an intrinsic rotation order.
 *
 * Intrinsic rotations after the first one are applied to the **rotated** coordinate axes.
 * For instance, intrinstic order XYZ first rotates around the X axis, then around the **rotated** Y
 * axis, then around the **rotated** Z axis.
 */
enum class IntrinsicRotationOrder
{
    xyz,
    xzy,
    yxz,
    yzx,
    zxy,
    zyx,
};

/**
 * Describes an extrinsic rotation order.
 *
 * Extrinsic rotations after the first one are applied to the **original** coordinate axes.
 * For instance, intrinstic order XYZ first rotates around the X axis, then around the **original**
 * Y axis, then around the **original** Z axis.
 */
enum class ExtrinsicRotationOrder
{
    xyz,
    xzy,
    yxz,
    yzx,
    zxy,
    zyx,
};

/**
 * Converts an extrinsic rotation order to its equivalent intrinsic rotation order.
 */
constexpr IntrinsicRotationOrder make_intrinsic(ExtrinsicRotationOrder rotation_order) noexcept
{
    // Extrinstic rotations can be turned into intrinstic rotations by flipping the order of
    // operations
    switch (rotation_order) {
    default:
        assert(false);
    case ExtrinsicRotationOrder::xyz:
        return IntrinsicRotationOrder::zyx;
    case ExtrinsicRotationOrder::xzy:
        return IntrinsicRotationOrder::yzx;
    case ExtrinsicRotationOrder::yxz:
        return IntrinsicRotationOrder::zxy;
    case ExtrinsicRotationOrder::yzx:
        return IntrinsicRotationOrder::xzy;
    case ExtrinsicRotationOrder::zxy:
        return IntrinsicRotationOrder::yxz;
    case ExtrinsicRotationOrder::zyx:
        return IntrinsicRotationOrder::xyz;
    }
}

/**
 * Converts an intrinsic rotation order to its equivalent extrinsic rotation order.
 */
constexpr ExtrinsicRotationOrder make_extrinsic(IntrinsicRotationOrder rotation_order) noexcept
{
    // Intrinstic rotations can be turned into extrinstic rotations by flipping the order of
    // operations
    switch (rotation_order) {
    default:
        assert(false);
    case IntrinsicRotationOrder::xyz:
        return ExtrinsicRotationOrder::zyx;
    case IntrinsicRotationOrder::xzy:
        return ExtrinsicRotationOrder::yzx;
    case IntrinsicRotationOrder::yxz:
        return ExtrinsicRotationOrder::zxy;
    case IntrinsicRotationOrder::yzx:
        return ExtrinsicRotationOrder::xzy;
    case IntrinsicRotationOrder::zxy:
        return ExtrinsicRotationOrder::yxz;
    case IntrinsicRotationOrder::zyx:
        return ExtrinsicRotationOrder::xyz;
    }
}

/**
 * \brief BasicQuaternion
 */
#pragma pack(push, 1)
template <typename ComponentT>
class BasicQuaternion final
{
public:
    /// The type of the quaternion's components
    using ComponentType = ComponentT;

    ComponentType x; ///< The quaternion's X element
    ComponentType y; ///< The quaternion's Y element
    ComponentType z; ///< The quaternion's Z element
    ComponentType w; ///< The quaternion's W element

    /// Constructs an uninitialized quaternion
    BasicQuaternion() noexcept = default;

    /// Constructs a quaternion from immediate floats
    BasicQuaternion(ComponentType fx, ComponentType fy, ComponentType fz, ComponentType fw) noexcept
        : x(fx), y(fy), z(fz), w(fw)
    {
    }

    /// Implicitly constructs the quaternion from another quaternion with a
    /// non-narrowing-convertible component
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicQuaternion(const BasicQuaternion<U>& q)
        : x(ComponentType{q.x}), y(ComponentType{q.y}), z(ComponentType{q.z}), w(ComponentType{q.w})
    {
    }

    /// Explicitly constructs the quaternion from another quaternion with a narrowing-convertible
    /// component type
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicQuaternion(const BasicQuaternion<U>& q) : x(q.x), y(q.y), z(q.z), w(q.w)
    {
    }

    /// Adds another quaternion to this quaternion
    BasicQuaternion& operator+=(const BasicQuaternion& q) noexcept
    {
        x += q.x;
        y += q.y;
        z += q.z;
        w += q.w;
        return *this;
    }

    /// Subtracts another quaternion from this quaternion
    BasicQuaternion& operator-=(const BasicQuaternion& q) noexcept
    {
        x -= q.x;
        y -= q.y;
        z -= q.z;
        w -= q.w;
        return *this;
    }

    /// Multiplies another quaternion with this quaternion
    BasicQuaternion& operator*=(const BasicQuaternion& q) noexcept
    {
        return *this = *this * q;
    }

    /// Scales the quaternion
    BasicQuaternion& operator*=(ComponentType s) noexcept
    {
        x *= s, y *= s, z *= s, w *= s;
        return *this;
    }

    /// Scales the quaternion (inverted)
    BasicQuaternion& operator/=(ComponentType s) noexcept
    {
        x /= s, y /= s, z /= s, w /= s;
        return *this;
    }

    /**
     * Indexes the quaternion.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     *
     * \throws std::out_of_range if the index is not in the range [0,3].
     */
    const ComponentT& operator[](std::size_t index) const
    {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        default:
            throw std::out_of_range("invalid BasicQuaternion subscript");
        }
    }

    /**
     * Indexes the quaternion.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     *
     * \throws std::out_of_range if the index is not in the range [0,3].
     */
    ComponentT& operator[](std::size_t index)
    {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        default:
            throw std::out_of_range("invalid BasicQuaternion subscript");
        }
    }

    /// Normalizes the quaternion
    void normalize() noexcept
    {
        const ComponentType inv_length = ComponentType(1.0) / length();
        x *= inv_length;
        y *= inv_length;
        z *= inv_length;
        w *= inv_length;
    }

    /// Checks if the quaternion is normalized
    [[nodiscard]] bool normalized() const noexcept
    {
        constexpr auto max_normalized_length = 0.000001;
        return abs(ComponentType(1.0) - length()) < max_normalized_length;
    }

    /// Calculates the length of the quaternion
    [[nodiscard]] ComponentType length() const noexcept
    {
        return sqrt(x * x + y * y + z * z + w * w);
    }

    /**
     \brief Calculates the squared length of the quaternion
     \details Calculating the squared length (length*length) is a considerably faster operation
        so use it whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] ComponentType length_sq() const noexcept
    {
        return x * x + y * y + z * z + w * w;
    }

    /// Calculates the dot product between this and the specified quaternion
    [[nodiscard]] ComponentType dot(const BasicQuaternion& v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }

    /// Converts the quaternion to a Euler rotation representation
    [[nodiscard]] BasicVector3<ComponentType> to_euler() const noexcept
    {
        return {-std::atan2(-2 * (y * z - w * x), 1 - 2 * (x * x + y * y)),
                -std::asin(2 * (x * z + w * y)),
                -std::atan2(-2 * (x * y - w * z), 1 - 2 * (y * y + z * z))};
    }

    /**
     * \brief Constructs a quaternion to represent a rotation around an axis
     * \param[in] axis The axis to rotate around
     * \param[in] angle The angle, in radians, to rotate around the axis
     */
    static BasicQuaternion from_axis_angle(const BasicVector3<ComponentType>& axis,
                                           ComponentType                      angle) noexcept
    {
        // Divide by axis' length to normalize it
        const float s = std::sin(angle / 2) / axis.length();
        return {axis.x * s, axis.y * s, axis.z * s, cos(angle / 2)};
    }

    /**
     * \brief Constructs a quaternion from _intrinsic_ Euler rotation angles
     *
     * \param[in] x Rotation, in radians, around the X-axis
     * \param[in] y Rotation, in radians, around the Y-axis
     * \param[in] z Rotation, in radians, around the Z-axis
     * \param[in] rotation_order the order of rotations
     */
    static BasicQuaternion from_euler(ComponentType x, ComponentType y, ComponentType z,
                                      IntrinsicRotationOrder rotation_order) noexcept
    {
        const auto sx = std::sin(x / 2);
        const auto cx = std::cos(x / 2);
        const auto sy = std::sin(y / 2);
        const auto cy = std::cos(y / 2);
        const auto sz = std::sin(z / 2);
        const auto cz = std::cos(z / 2);

        switch (rotation_order) {
        default:
        case IntrinsicRotationOrder::xyz:
            return {sx * cy * cz + cx * sy * sz, cx * sy * cz - sx * cy * sz,
                    cx * cy * sz + sx * sy * cz, cx * cy * cz - sx * sy * sz};

        case IntrinsicRotationOrder::xzy:
            return {sx * cy * cz - cx * sy * sz, cx * sy * cz - sx * cy * sz,
                    cx * cy * sz + sx * sy * cz, cx * cy * cz + sx * sy * sz};

        case IntrinsicRotationOrder::yxz:
            return {sx * cy * cz + cx * sy * sz, cx * sy * cz - sx * cy * sz,
                    cx * cy * sz - sx * sy * cz, cx * cy * cz + sx * sy * sz};

        case IntrinsicRotationOrder::yzx:
            return {sx * cy * cz + cx * sy * sz, cx * sy * cz + sx * cy * sz,
                    cx * cy * sz - sx * sy * cz, cx * cy * cz - sx * sy * sz};

        case IntrinsicRotationOrder::zxy:
            return {sx * cy * cz - cx * sy * sz, cx * sy * cz + sx * cy * sz,
                    cx * cy * sz + sx * sy * cz, cx * cy * cz - sx * sy * sz};

        case IntrinsicRotationOrder::zyx:
            return {sx * cy * cz - cx * sy * sz, cx * sy * cz + sx * cy * sz,
                    cx * cy * sz - sx * sy * cz, cx * cy * cz + sx * sy * sz};
        }
    }

    /**
     * \brief Constructs a quaternion from _extrinsic_ Euler rotation angles
     *
     * \param[in] x Rotation, in radians, around the X-axis
     * \param[in] y Rotation, in radians, around the Y-axis
     * \param[in] z Rotation, in radians, around the Z-axis
     * \param[in] rotation_order the order of rotations
     */
    static BasicQuaternion from_euler(ComponentType x, ComponentType y, ComponentType z,
                                      ExtrinsicRotationOrder rotation_order) noexcept
    {
        return from_euler(x, y, z, make_intrinsic(rotation_order));
    }

    /// Identity quaternion
    static const BasicQuaternion IDENTITY;
};
#pragma pack(pop)

template <typename T>
const BasicQuaternion<T> BasicQuaternion<T>::IDENTITY(0, 0, 0, 1);

/// Quaternion of doubles
using Quaternion = BasicQuaternion<double>;

/// Quaternion of floats
using Quaternionf = BasicQuaternion<float>;

/// Validate that the vector has the expected size, because this type can be directly used in a
/// mapping to graphics engine's memory.
static_assert(sizeof(Quaternion) == 4 * sizeof(Quaternion::ComponentType),
              "BasicQuaternion does not have the expected size");

/// Adds quaternion \a q2 to quaternion \a q1
template <typename T>
auto operator+(const BasicQuaternion<T>& q1, const BasicQuaternion<T>& q2) noexcept
{
    return BasicQuaternion<T>(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
}

/// Subtracts quaternion \a q2 from quaternion \a q1
template <typename T>
auto operator-(const BasicQuaternion<T>& q1, const BasicQuaternion<T>& q2) noexcept
{
    return BasicQuaternion<T>(q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w);
}

/// Scales quaternion \a q with scalar \a s
template <typename T>
auto operator*(const BasicQuaternion<T>& q, T s) noexcept
{
    return BasicQuaternion<T>(q.x * s, q.y * s, q.z * s, q.w * s);
}

/// Scales quaternion \a q with scalar \a s
template <typename T>
auto operator*(T s, const BasicQuaternion<T>& q) noexcept
{
    return BasicQuaternion<T>(q.x * s, q.y * s, q.z * s, q.w * s);
}

template <typename T>
BasicQuaternion<T> operator*(const BasicQuaternion<T>& q1, const BasicQuaternion<T>& q2) noexcept
{
    return {q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
            q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
            q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z};
}

/// Scales quaternion \a q with scalar 1/\a s
template <typename T>
auto operator/(const BasicQuaternion<T>& q, T s) noexcept
{
    return BasicQuaternion<T>(q.x / s, q.y / s, q.z / s, q.w / s);
}

/// Computes the dot-product of quaternion \a q1 and quaternion \a q2
template <typename T>
auto dot(const BasicQuaternion<T>& q1, const BasicQuaternion<T>& q2) noexcept
{
    return q1.dot(q2);
}

/// Normalizes quaternion \a q
template <typename T>
auto normalize(const BasicQuaternion<T>& q) noexcept
{
    const auto inv_length = T{1.0} / q.length();
    return BasicQuaternion<T>(q.x * inv_length, q.y * inv_length, q.z * inv_length,
                              q.w * inv_length);
}

/// Transforms (post-multiplies) a vector with a rotation quaternion
template <typename T>
BasicVector3<T> operator*(const BasicVector3<T>& v, const BasicQuaternion<T>& q) noexcept
{
    // Optimized version of Matrix(q).transform_coord(v)
    const BasicVector3<T> qv(q.x, q.y, q.z);
    const BasicVector3<T> t = qv.cross(v) * 2;
    return v + t * q.w + qv.cross(t);
}

/// Transforms (post-multiplies) a vector with a rotation quaternion
template <typename T>
BasicVector4<T> operator*(const BasicVector4<T>& v, const BasicQuaternion<T>& q) noexcept
{
    // Apply the transformation to the XYZ components of the vector and leave W untouched.
    return {BasicVector3(v) * q, v.w};
}

/**
 * \brief spherical linear interpolation between quaternions.
 *
 * \note for performance reasons this implementation does not do true spherical linear
 * implementation, but instead does linear interpolation. This will generally make rotations look
 * good enough.
 */
template <typename T>
BasicQuaternion<T> slerp(const BasicQuaternion<T>& v0, const BasicQuaternion<T>& v1, T t) noexcept
{
    // Not really spherical linear interpolation.
    // Just linear interpolation with a possible sign-flip so rotations
    // look good (shortest path).
    const auto d = v0.dot(v1);
    return v0 * (1 - t) + v1 * t * (d < 0 ? -1.0 : 1.0);
}

/// Computes the inverse of quaternion \a q
template <typename T>
auto inverse(const BasicQuaternion<T>& q) noexcept
{
    const auto inv_length = T{1.0} / q.length_sq();
    return BasicQuaternion<T>(-q.x, -q.y, -q.z, q.w) * inv_length;
}

} // namespace khepri
