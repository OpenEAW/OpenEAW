#pragma once

#include "math.hpp"

#include <khepri/utility/type_traits.hpp>

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <cmath>
#include <type_traits>

namespace khepri {

template <typename ComponentT>
class BasicVector3;

template <typename ComponentT>
class BasicVector4;

/**
 * \brief A 2-component vector.
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
class BasicVector2 final
{
public:
    /// The type of the vector's components
    using ComponentType = ComponentT;

    ComponentType x{}; ///< The vector's X element
    ComponentType y{}; ///< The vector's Y element

    /// Constructs an uninitialized vector
    constexpr BasicVector2() noexcept = default;

    /// Constructs the vector from literal floats
    constexpr BasicVector2(ComponentType fx, ComponentType fy) noexcept : x(fx), y(fy) {}

    /// Implicitly constructs the vector from another vector with a non-narrowing-convertible
    /// component
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector2(const BasicVector2<U>& v) : x(ComponentType{v.x}), y(ComponentType{v.y})
    {
    }

    /// Explicitly constructs the vector from another vector with a narrowing-convertible component
    /// type
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector2(const BasicVector2<U>& v) : x(v.x), y(v.y)
    {
    }

    /// Implicitly constructs the vector from a BasicVector3 with a non-narrowing-convertible
    /// component type by throwing away the Z component.
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector2(const BasicVector3<U>& v);

    /// Explicitly constructs the vector from a BasicVector3 with a narrowing-convertible component
    /// type by throwing away the Z component.
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector2(const BasicVector3<U>& v);

    /// Implicitly constructs the vector from a BasicVector4 with a non-narrowing-convertible
    /// component type by throwing away the Z and W components.
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector2(const BasicVector4<U>& v);

    /// Explicitly constructs the vector from a BasicVector4 with a narrowing-convertible component
    /// type by throwing away the Z and W components.
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector2(const BasicVector4<U>& v);

    /// Adds vector \a v to the vector
    BasicVector2& operator+=(const BasicVector2& v) noexcept
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    /// Subtracts vector \a v from the vector
    BasicVector2& operator-=(const BasicVector2& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    /// Scales the vector by \a s
    BasicVector2& operator*=(ComponentType s) noexcept
    {
        x *= s;
        y *= s;
        return *this;
    }

    /// Scales the vector with 1.0 / \a s
    BasicVector2& operator/=(ComponentType s) noexcept
    {
        x /= s;
        y /= s;
        return *this;
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y.
     */
    const ComponentType& operator[](std::size_t index) const noexcept
    {
        assert(index < 2);
        return gsl::span<const ComponentType>(&x, 2)[index];
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y.
     */
    ComponentType& operator[](std::size_t index) noexcept
    {
        assert(index < 2);
        return gsl::span<ComponentType>(&x, 2)[index];
    }

    /// Calculates the length of the vector
    [[nodiscard]] ComponentType length() const noexcept
    {
        return sqrt(x * x + y * y);
    }

    /**
     * \brief Calculates the squared length of the vector
     *
     * Calculating the squared length (length*length) is a considerably faster operation so use it
     * whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] ComponentType length_sq() const noexcept
    {
        return x * x + y * y;
    }

    /// Calculates the distance between the vector and vector \a v
    template <typename U>
    [[nodiscard]] auto distance(const BasicVector2<U>& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        return sqrt(dx * dx + dy * dy);
    }

    /**
     * \brief Calculates the squared distance between the vector and vector \a v
     *
     * Calculating the squared distance (distance*distance) is a considerably faster operation so
     * use it whenever possible (e.g., when comparing distances)
     */
    template <typename U>
    [[nodiscard]] auto distance_sq(const BasicVector2<U>& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        return dx * dx + dy * dy;
    }

    /// Calculates the angle that the vector makes with the positive X axis
    [[nodiscard]] ComponentType angle() const noexcept
    {
        return atan2(y, x);
    }

    /// Calculates the dot product between the vector and vector \a v
    template <typename U>
    [[nodiscard]] auto dot(const BasicVector2<U>& v) const noexcept
    {
        return x * v.x + y * v.y;
    }

    /// Calculates the cross product between the vector and vector \a v
    template <typename U>
    [[nodiscard]] auto cross(const BasicVector2<U>& v) const noexcept
    {
        return x * v.y - y * v.x;
    }

    /// Normalizes the vector
    void normalize() noexcept
    {
        auto inv_length = ComponentType{1.0} / length();
        x *= inv_length;
        y *= inv_length;
    }

    /// Checks if the vector is normalized
    [[nodiscard]] bool normalized() const noexcept
    {
        constexpr auto max_normalized_length = 0.000001;
        return abs(1.0F - length()) < max_normalized_length;
    }

    /// Constructs a unit vector from an angle with the positive X-axis
    static BasicVector2 from_angle(ComponentType phi) noexcept
    {
        return BasicVector2(cos(phi), sin(phi));
    }
};
#pragma pack(pop)

/// 2D vector of doubles
using Vector2 = BasicVector2<double>;

/// 2D vector floats
using Vector2f = BasicVector2<float>;

/// Validate that the vector has the expected size, because this type can be directly used in a
/// mapping to graphics engine's memory.
static_assert(sizeof(Vector2) == 2 * sizeof(Vector2::ComponentType),
              "BasicVector2 does not have the expected size");

/// Negates vector \a v
template <typename T>
auto operator-(const BasicVector2<T>& v) noexcept
{
    return BasicVector2<T>(-v.x, -v.y);
}

/// Adds vector \a v2 to vector \a v1
template <typename T, typename U>
auto operator+(const BasicVector2<T>& v1, const BasicVector2<U>& v2) noexcept
{
    return BasicVector2<std::common_type_t<T, U>>(v1.x + v2.x, v1.y + v2.y);
}

/// Subtracts vector \a v2 from vector \a v1
template <typename T, typename U>
auto operator-(const BasicVector2<T>& v1, const BasicVector2<U>& v2) noexcept
{
    return BasicVector2<std::common_type_t<T, U>>(v1.x - v2.x, v1.y - v2.y);
}

/// Scales vector \a v with scalar \a s
template <typename T, typename U>
auto operator*(const BasicVector2<T>& v, U s) noexcept
{
    return BasicVector2<std::common_type_t<T, U>>(v.x * s, v.y * s);
}

/// Scales vector \a v with scalar \a s
template <typename T, typename U>
auto operator*(T s, const BasicVector2<U>& v) noexcept
{
    return BasicVector2<std::common_type_t<T, U>>(v.x * s, v.y * s);
}

/// Scales vector \a v with scalar 1/\a s
template <typename T, typename U>
auto operator/(const BasicVector2<T>& v, U s) noexcept
{
    return BasicVector2<std::common_type_t<T, U>>(v.x / s, v.y / s);
}

/// Multiplies vector \a v1 with vector \a v2, component-wise
template <typename T, typename U>
auto operator*(const BasicVector2<T>& v1, const BasicVector2<U>& v2) noexcept
{
    return BasicVector2<std::common_type_t<T, U>>(v1.x * v2.x, v1.y * v2.y);
}

/// Calculates the distance between the points identified by vector \a v1 and vector \a v2
template <typename T, typename U>
auto distance(const BasicVector2<T>& v1, const BasicVector2<U>& v2) noexcept
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
auto distance_sq(const BasicVector2<T>& v1, const BasicVector2<U>& v2) noexcept
{
    return v1.distance_sq(v2);
}

/// Calculates the dot product between vector \a v1 and vector \a v2
template <typename T, typename U>
auto dot(const BasicVector2<T>& v1, const BasicVector2<U>& v2) noexcept
{
    return v1.dot(v2);
}

/// Calculates the cross product between vector \a v1 and vector \a v2
template <typename T, typename U>
auto cross(const BasicVector2<T>& v1, const BasicVector2<U>& v2) noexcept
{
    return v1.cross(v2);
}

/// Normalizes vector \a v
template <typename T>
auto normalize(const BasicVector2<T>& v) noexcept
{
    auto invL = T{1.0} / v.length();
    return BasicVector2<T>(v.x * invL, v.y * invL);
}

/**
 * \brief Rotates the vector around the origin.
 *
 * A positive angle rotates +x towards -y.
 *
 * \param v     the vector to rotate.
 * \param angle the angle to rotate by, in radians.
 * \return the rotated vector
 */
template <typename T, typename U>
auto rotate(const BasicVector2<T>& v, U angle) noexcept
{
    const T s = std::sin(angle);
    const T c = std::cos(angle);
    return BasicVector2<T>{c * v.x - s * v.y, s * v.x + c * v.y};
}

/**
 * \brief Clamps each component of a vector between two extremes
 *
 * Returns \a min if \a val.{x,y} < \a min.
 * Returns \a max if \a val.{x,y} > \a max.
 * Otherwise, returns \a val.{x,y}.
 *
 * \param[in] val the vector to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val, with each component clamped between \a min and \a max.
 */
template <typename T, typename U>
constexpr auto clamp(const BasicVector2<T>& val, U min, U max) noexcept
{
    return BasicVector2<T>{clamp(val.x, min, max), clamp(val.y, min, max)};
}

/**
 * \brief Clamps each component of a vector between 0 and 1
 *
 * Returns \a 0 if \a val.{x,y} < \a 0.
 * Returns \a 1 if \a val.{x,y} > \a 1.
 * Otherwise, returns \a val.{x,y}.
 *
 * \param[in] val the value to clamp
 *
 * \return \a val clamped between 0 and 1.
 */
template <typename T>
constexpr auto saturate(const BasicVector2<T>& val) noexcept
{
    return clamp(val, T{0}, T{1});
}

} // namespace khepri
