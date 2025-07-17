#pragma once

#include "math.hpp"
#include "vector2.hpp"

#include <khepri/utility/type_traits.hpp>

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <cmath>
#include <type_traits>

namespace khepri {

template <typename ComponentT>
class BasicVector4;

/**
 * \brief A 3-component vector.
 *
 * A vector is a displacement in a Euclidian space. Unlike a \see point, a vector has a magnitude
 * and a direction. As such, a vector supports operation such as getting the angle between two
 * vectors, getting a vector's length, rotating a vector, and so on.
 *
 * Vectors and points can be converted by explicitly casting from one to the other.
 *
 * \tparam ComponentT specifies the type the vector's components (default = double).
 */
#pragma pack(push, 1)
template <typename ComponentT = double>
class BasicVector3 final
{
public:
    /// The type of the vector's components
    using ComponentType = ComponentT;

    ComponentType x{}; ///< The vector's X element
    ComponentType y{}; ///< The vector's Y element
    ComponentType z{}; ///< The vector's Z element

    /// Constructs an uninitialized vector
    constexpr BasicVector3() noexcept = default;

    /// Constructs the vector from literals
    constexpr BasicVector3(ComponentType fx, ComponentType fy, ComponentType fz) noexcept
        : x(fx), y(fy), z(fz)
    {
    }

    /// Implicitly constructs the vector from another vector with a non-narrowing-convertible
    /// component
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector3(const BasicVector3<U>& v)
        : x(ComponentType{v.x}), y(ComponentType{v.y}), z(ComponentType{v.z})
    {
    }

    /// Explicitly constructs the vector from another vector with a narrowing-convertible component
    /// type
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector3(const BasicVector3<U>& v) : x(v.x), y(v.y), z(v.z)
    {
    }

    /// Implicitly constructs the vector from a Vector2 with a non-narrowing-convertible component
    /// type, and a Z component
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector3(const BasicVector2<U>& v, ComponentType z)
        : x(ComponentType{v.x}), y(ComponentType{v.y}), z(z)
    {
    }

    /// Explicitly constructs the vector from a Vector2 with a narrowing-convertible component
    /// type, and a Z component
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector3(const BasicVector2<U>& v, ComponentType z)
        : x(v.x), y(v.y), z(z)
    {
    }

    /// Implicitly constructs the vector from a Vector4 with a non-narrowing-convertible component
    /// type by throwing away the W component.
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector3(const BasicVector4<U>& v);

    /// Explicitly constructs the vector from a Vector4 with a narrowing-convertible component
    /// type by throwing away the W component.
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector3(const BasicVector4<U>& v);

    /// Adds vector \a v to the vector
    BasicVector3& operator+=(const BasicVector3& v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    /// Subtracts vector \a v from the vector
    BasicVector3& operator-=(const BasicVector3& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    /// Scales the vector by scalar \a s
    BasicVector3& operator*=(ComponentType s) noexcept
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    /// Scales the vector with scalar 1 / \a s
    BasicVector3& operator/=(ComponentType s) noexcept
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    const ComponentType& operator[](std::size_t index) const noexcept
    {
        assert(index < 3);
        return gsl::span<const ComponentType>(&x, 3)[index];
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    ComponentType& operator[](std::size_t index) noexcept
    {
        assert(index < 3);
        return gsl::span<ComponentType>(&x, 3)[index];
    }

    /// Calculates the length of the vector
    [[nodiscard]] ComponentType length() const noexcept
    {
        return sqrt(x * x + y * y + z * z);
    }

    /**
     * \brief Calculates the squared length of the vector
     *
     * Calculating the squared length (length*length) is a considerably faster operation so use it
     * whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] ComponentType length_sq() const noexcept
    {
        return x * x + y * y + z * z;
    }

    /// Calculates the distance between the vector and vector \a v
    template <typename U>
    [[nodiscard]] auto distance(const BasicVector3<U>& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        return sqrt(dx * dx + dy * dy + dz * dz);
    }

    /**
     * \brief Calculates the squared distance between the vector and vector \a v
     *
     * Calculating the squared distance (distance*distance) is a considerably faster operation so
     * use it whenever possible (e.g., when comparing distances)
     */
    template <typename U>
    [[nodiscard]] auto distance_sq(const BasicVector3<U>& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        return dx * dx + dy * dy + dz * dz;
    }

    /// Calculates the dot product between the vector and vector \a v
    template <typename U>
    [[nodiscard]] auto dot(const BasicVector3<U>& v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z;
    }

    /// Interprets the vector as a point on a sphere and returns the tilt (angle above the XY
    /// plane), in radians
    [[nodiscard]] ComponentType tilt() const noexcept
    {
        return atan2(z, sqrt(x * x + y * y));
    }

    /// Interprets the vector as a point on a sphere and returns the Z-angle (angle around Z-axis),
    /// in radians \note 0 radians is the X axis and 1/2 PI radians is the Y axis
    [[nodiscard]] ComponentType z_angle() const noexcept
    {
        return atan2(y, x);
    }

    /// Calculates the cross product between the vector and vector \a v
    template <typename U>
    [[nodiscard]] auto cross(const BasicVector3<U>& v) const noexcept
    {
        return BasicVector3<std::common_type_t<ComponentType, U>>(
            y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    /// Normalizes the vector
    void normalize() noexcept
    {
        const auto inv_length = 1.0F / length();
        x *= inv_length;
        y *= inv_length;
        z *= inv_length;
    }

    /// Checks if the vector is normalized
    [[nodiscard]] bool normalized() const noexcept
    {
        constexpr auto max_normalized_length = 0.000001;
        return abs(1.0F - length()) < max_normalized_length;
    }

    /// Constructs a unit vector from a tilt (angle above the XY plane) and Z-angle (angle around
    /// Z-axis), both in radians.
    /// \note For Z-angle, 0 radians is the X axis and 1/2 PI radians is the Y axis.
    [[nodiscard]] static BasicVector3 from_angles(double tilt, double z_angle) noexcept
    {
        const auto r = std::cos(tilt);
        return {r * std::cos(z_angle), r * std::sin(z_angle), std::sin(tilt)};
    }
};
#pragma pack(pop)

/// 3D vector of doubles
using Vector3 = BasicVector3<double>;

/// 3D vector of floats
using Vector3f = BasicVector3<float>;

/// Validate that the vector has the expected size, because this type can be directly used in a
/// mapping to graphics engine's memory.
static_assert(sizeof(Vector3) == 3 * sizeof(Vector3::ComponentType),
              "BasicVector3 does not have the expected size");

/// Negates vector \a v
template <typename T>
auto operator-(const BasicVector3<T>& v) noexcept
{
    return BasicVector3<T>(-v.x, -v.y, -v.z);
}

/// Adds vector \a v2 to vector \a v1
template <typename T, typename U>
auto operator+(const BasicVector3<T>& v1, const BasicVector3<U>& v2) noexcept
{
    return BasicVector3<std::common_type_t<T, U>>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

/// Subtracts vector \a v2 from vector \a v1
template <typename T, typename U>
auto operator-(const BasicVector3<T>& v1, const BasicVector3<U>& v2) noexcept
{
    return BasicVector3<std::common_type_t<T, U>>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

/// Scales vector \a v with scalar \a s
template <typename T, typename U>
auto operator*(const BasicVector3<T>& v, U s) noexcept
{
    return BasicVector3<std::common_type_t<T, U>>(v.x * s, v.y * s, v.z * s);
}

/// Scales vector \a v with scalar \a s
template <typename T, typename U>
auto operator*(T s, const BasicVector3<U>& v) noexcept
{
    return BasicVector3<std::common_type_t<T, U>>(v.x * s, v.y * s, v.z * s);
}

/// Scales vector \a v with scalar 1/\a s
template <typename T, typename U>
auto operator/(const BasicVector3<T>& v, U s) noexcept
{
    return BasicVector3<std::common_type_t<T, U>>(v.x / s, v.y / s, v.z / s);
}

/// Multiplies vector \a v1 with vector \a v2, component-wise
template <typename T, typename U>
auto operator*(const BasicVector3<T>& v1, const BasicVector3<U>& v2) noexcept
{
    return BasicVector3<std::common_type_t<T, U>>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

/// Calculates the distance between the points identified by vector \a v1 and vector \a v2
template <typename T, typename U>
auto distance(const BasicVector3<T>& v1, const BasicVector3<U>& v2) noexcept
{
    return v1.distance(v2);
}

/**
 * \brief Calculates the squared distance between the points identified by vector \a v1 and vector
 *        \a v2
 *
 * Calculating the squared distance (distance*distance) is a considerably faster operation so
 * use it whenever possible (e.g., when comparing distances)
 */
template <typename T, typename U>
auto distance_sq(const BasicVector3<T>& v1, const BasicVector3<U>& v2) noexcept
{
    return v1.distance_sq(v2);
}

/// Calculates the dot product between vector \a v1 and vector \a v2
template <typename T, typename U>
auto dot(const BasicVector3<T>& v1, const BasicVector3<U>& v2) noexcept
{
    return v1.dot(v2);
}

/// Calculates the cross product between vector \a v1 and vector \a v2
template <typename T, typename U>
auto cross(const BasicVector3<T>& v1, const BasicVector3<U>& v2) noexcept
{
    return v1.cross(v2);
}

/// Normalizes vector \a v
template <typename T>
auto normalize(const BasicVector3<T>& v) noexcept
{
    BasicVector3<T> nv(v);
    nv.normalize();
    return nv;
}

/// Implicitly constructs the vector from a BasicVector3 with a non-narrowing-convertible component
/// type by throwing away the Z component.
template <typename ComponentT>
template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentT> &&
                                                    !is_narrowing_conversion_v<U, ComponentT>,
                                                void*>>
constexpr BasicVector2<ComponentT>::BasicVector2(const BasicVector3<U>& v)
    : x(ComponentT{v.x}), y(ComponentT{v.y})
{
}

/// Explicitly constructs the vector from a BasicVector3 with a narrowing-convertible component
/// type by throwing away the Z component.
template <typename ComponentT>
template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentT> &&
                                                    is_narrowing_conversion_v<U, ComponentT>,
                                                void*>>
constexpr BasicVector2<ComponentT>::BasicVector2(const BasicVector3<U>& v) : x(v.x), y(v.y)
{
}

/**
 * Checks if two vectors are colinear.
 *
 * Two vectors are colinear if they point in the same direction. Opposite vectors are also
 * considered colinear.
 *
 * \note as with all floating-point checks, there is a tiny error margin
 */
template <typename T, typename U>
bool colinear(const BasicVector3<T>& v1, const BasicVector3<U>& v2) noexcept
{
    constexpr auto max_colinear_sq_length = 0.000001;
    return cross(v1, v2).length_sq() < max_colinear_sq_length;
}

/**
 * \brief Clamps each component of a vector between two extremes
 *
 * Returns \a min if \a val.{x,y,z} < \a min.
 * Returns \a max if \a val.{x,y,z} > \a max.
 * Otherwise, returns \a val.{x,y,z}.
 *
 * \param[in] val the vector to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val, with each component clamped between \a min and \a max.
 */
template <typename T, typename U>
constexpr auto clamp(const BasicVector3<T>& val, U min, U max) noexcept
{
    return BasicVector3<T>{clamp(val.x, min, max), clamp(val.y, min, max), clamp(val.z, min, max)};
}

/**
 * \brief Clamps each component of a vector between 0 and 1
 *
 * Returns \a 0 if \a val.{x,y,z} < \a 0.
 * Returns \a 1 if \a val.{x,y,z} > \a 1.
 * Otherwise, returns \a val.{x,y,z}.
 *
 * \param[in] val the value to clamp
 *
 * \return \a val clamped between 0 and 1.
 */
template <typename T>
constexpr auto saturate(const BasicVector3<T>& val) noexcept
{
    return clamp(val, T{0}, T{1});
}

} // namespace khepri
