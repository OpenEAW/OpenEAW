#pragma once

#include "math.hpp"
#include "vector3.hpp"

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <cmath>
#include <type_traits>

namespace khepri {

class matrix;

/**
 * \brief A 4-component vector.
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
class BasicVector4 final
{
public:
    /// The type of the vector's components
    using ComponentType = ComponentT;

    ComponentType x{}; ///< The vector's X element
    ComponentType y{}; ///< The vector's Y element
    ComponentType z{}; ///< The vector's Z element
    ComponentType w{}; ///< The vector's W element

    /// Constructs an uninitialized vector
    constexpr BasicVector4() noexcept = default;

    /// Constructs the vector from literal floats
    constexpr BasicVector4(ComponentType fx, ComponentType fy, ComponentType fz,
                           ComponentType fw) noexcept
        : x(fx), y(fy), z(fz), w(fw)
    {
    }

    /// Implicitly constructs the vector from another vector with a non-narrowing-convertible
    /// component
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector4(const BasicVector4<U>& v)
        : x(ComponentType{v.x}), y(ComponentType{v.y}), z(ComponentType{v.z}), w(ComponentType{v.w})
    {
    }

    /// Explicitly constructs the vector from another vector with a narrowing-convertible component
    /// type
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector4(const BasicVector4<U>& v) : x(v.x), y(v.y), z(v.z), w(v.w)
    {
    }

    /// Implicitly constructs the vector from a BasicVector2 with a non-narrowing-convertible
    /// component type, and a Z and W component
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector4(const BasicVector2<U>& v, ComponentType z, ComponentType w)
        : x(ComponentType{v.x}), y(ComponentType{v.y}), z(z), w(w)
    {
    }

    /// Explicitly constructs the vector from a BasicVector2 with a narrowing-convertible component
    /// type, and a Z and W component
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector4(const BasicVector2<U>& v, ComponentType z, ComponentType w)
        : x(v.x), y(v.y), z(z), w(w)
    {
    }

    /// Implicitly constructs the vector from a BasicVector3 with a non-narrowing-convertible
    /// component type, and a w component
    template <typename U,
              typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                            !is_narrowing_conversion_v<U, ComponentType>,
                                        void*> = nullptr>
    constexpr BasicVector4(const BasicVector3<U>& v, ComponentType w)
        : x(ComponentType{v.x}), y(ComponentType{v.y}), z(ComponentType{v.z}), w(w)
    {
    }

    /// Explicitly constructs the vector from a BasicVector3 with a narrowing-convertible component
    /// type, and a W component
    template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentType> &&
                                                        is_narrowing_conversion_v<U, ComponentType>,
                                                    void*> = nullptr>
    explicit constexpr BasicVector4(const BasicVector3<U>& v, ComponentType w)
        : x(v.x), y(v.y), z(v.z), w(w)
    {
    }

    /// Adds vector \a v to the vector
    constexpr BasicVector4& operator+=(const BasicVector4& v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

    /// Subtracts vector \a v from the vector
    constexpr BasicVector4& operator-=(const BasicVector4& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

    /// Scales the vector by scalar \a s
    constexpr BasicVector4& operator*=(ComponentType s) noexcept
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }

    /// Scales the vector by scalar 1.0 / \a s
    constexpr BasicVector4& operator/=(ComponentType s) noexcept
    {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    constexpr const ComponentType& operator[](std::size_t index) const noexcept
    {
        assert(index < 4);
        return gsl::span<const ComponentType>(&x, 4)[index];
    }

    /**
     * Indexes the vector.
     *
     * \param[in] index the component index to return. 0 is X, 1 is Y, etc.
     */
    constexpr ComponentType& operator[](std::size_t index) noexcept
    {
        assert(index < 4);
        return gsl::span<ComponentType>(&x, 4)[index];
    }

    /// Calculates the length of the vector
    [[nodiscard]] ComponentType length() const noexcept
    {
        return std::sqrt(length_sq());
    }

    /**
     * \brief Calculates the squared length of the vector
     *
     * Calculating the squared length (length*length) is a considerably faster operation so use it
     * whenever possible (e.g., when comparing lengths)
     */
    [[nodiscard]] constexpr ComponentType length_sq() const noexcept
    {
        return x * x + y * y + z * z + w * w;
    }

    /// Calculates the distance between the vector and vector \a v
    template <typename U>
    [[nodiscard]] auto distance(const BasicVector4<U>& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        const auto dw = v.w - w;
        return std::sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
    }

    /**
     * \brief Calculates the squared distance between the vector and vector \a v
     *
     * Calculating the squared distance (distance*distance) is a considerably faster operation so
     * use it whenever possible (e.g., when comparing distances)
     */
    template <typename U>
    [[nodiscard]] constexpr auto distance_sq(const BasicVector4<U>& v) const noexcept
    {
        const auto dx = v.x - x;
        const auto dy = v.y - y;
        const auto dz = v.z - z;
        const auto dw = v.w - w;
        return dx * dx + dy * dy + dz * dz + dw * dw;
    }

    /// Calculates the dot product between the vector and vector \a v
    template <typename U>
    [[nodiscard]] constexpr auto dot(const BasicVector4<U>& v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }

    /// Normalizes the vector
    void normalize() noexcept
    {
        const ComponentType inv_length = 1.0F / length();
        x *= inv_length;
        y *= inv_length;
        z *= inv_length;
        w *= inv_length;
    }

    /// Checks if the vector is normalized
    [[nodiscard]] bool normalized() const noexcept
    {
        constexpr auto max_normalized_length = 0.000001;
        return abs(1.0F - length()) < max_normalized_length;
    }
};
#pragma pack(pop)

/// 4D vector of doubles
using Vector4 = BasicVector4<double>;

/// 4D vector of floats
using Vector4f = BasicVector4<float>;

/// Validate that the vector has the expected size, because this type can be directly used in a
/// mapping to graphics engine's memory.
static_assert(sizeof(Vector4f) == 4 * sizeof(Vector4f::ComponentType),
              "BasicVector4 does not have the expected size");

/// Negates vector \a v
template <typename T>
constexpr auto operator-(const BasicVector4<T>& v) noexcept
{
    return BasicVector4<T>(-v.x, -v.y, -v.z, -v.w);
}

/// Adds vector \a v2 to vector \a v1
template <typename T, typename U>
constexpr auto operator+(const BasicVector4<T>& v1, const BasicVector4<U>& v2) noexcept
{
    return BasicVector4<std::common_type_t<T, U>>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z,
                                                  v1.w + v2.w);
}

/// Subtracts vector \a v2 from vector \a v1
template <typename T, typename U>
constexpr auto operator-(const BasicVector4<T>& v1, const BasicVector4<U>& v2) noexcept
{
    return BasicVector4<std::common_type_t<T, U>>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z,
                                                  v1.w - v2.w);
}

/// Scales vector \a v with scalar \a s
template <typename T, typename U>
constexpr auto operator*(const BasicVector4<T>& v, U s) noexcept
{
    return BasicVector4<std::common_type_t<T, U>>(v.x * s, v.y * s, v.z * s, v.w * s);
}

/// Scales vector \a v with scalar \a s
template <typename T, typename U>
constexpr auto operator*(T s, const BasicVector4<U>& v) noexcept
{
    return BasicVector4<std::common_type_t<T, U>>(v.x * s, v.y * s, v.z * s, v.w * s);
}

/// Scales vector \a v with scalar 1/\a s
template <typename T, typename U>
constexpr auto operator/(const BasicVector4<T>& v, U s) noexcept
{
    return BasicVector4<std::common_type_t<T, U>>(v.x / s, v.y / s, v.z / s, v.w / s);
}

/// Multiplies vector \a v1 with vector \a v2, component-wise
template <typename T, typename U>
constexpr auto operator*(const BasicVector4<T>& v1, const BasicVector4<U>& v2) noexcept
{
    return BasicVector4<std::common_type_t<T, U>>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z,
                                                  v1.w * v2.w);
}

/// Calculates the distance between the points identified by vector \a v1 and vector \a v2
template <typename T, typename U>
auto distance(const BasicVector4<T>& v1, const BasicVector4<U>& v2) noexcept
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
constexpr auto distance_sq(const BasicVector4<T>& v1, const BasicVector4<U>& v2) noexcept
{
    return v1.distance_sq(v2);
}

/// Calculates the dot product between vector \a v1 and vector \a v2
template <typename T, typename U>
constexpr auto dot(const BasicVector4<T>& v1, const BasicVector4<U>& v2) noexcept
{
    return v1.dot(v2);
}

/// Normalizes vector \a v
template <typename T>
auto normalize(const BasicVector4<T>& v) noexcept
{
    return v / v.length();
}

template <typename ComponentT>
template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentT> &&
                                                    !is_narrowing_conversion_v<U, ComponentT>,
                                                void*>>
constexpr BasicVector2<ComponentT>::BasicVector2(const BasicVector4<U>& v)
    : x(ComponentT{v.x}), y(ComponentT{v.y})
{
}

/// Explicitly constructs the vector from a BasicVector4 with a narrowing-convertible component
/// type by throwing away the Z and W components.
template <typename ComponentT>
template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentT> &&
                                                    is_narrowing_conversion_v<U, ComponentT>,
                                                void*>>
constexpr BasicVector2<ComponentT>::BasicVector2(const BasicVector4<U>& v) : x(v.x), y(v.y)
{
}

/// Implicitly constructs the vector from a BasicVector4 with a non-narrowing-convertible component
/// type by throwing away the W component.
template <typename ComponentT>
template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentT> &&
                                                    !is_narrowing_conversion_v<U, ComponentT>,
                                                void*>>
constexpr BasicVector3<ComponentT>::BasicVector3(const BasicVector4<U>& v)
    : x(ComponentT{v.x}), y(ComponentT{v.y}), z(ComponentT{v.z})
{
}

/// Explicitly constructs the vector from a BasicVector4 with a narrowing-convertible component
/// type by throwing away the W component.
template <typename ComponentT>
template <typename U, typename std::enable_if_t<std::is_convertible_v<U, ComponentT> &&
                                                    is_narrowing_conversion_v<U, ComponentT>,
                                                void*>>
constexpr BasicVector3<ComponentT>::BasicVector3(const BasicVector4<U>& v) : x(v.x), y(v.y), z(v.z)
{
}

/**
 * \brief Clamps each component of a vector between two extremes
 *
 * Returns \a min if \a val.{x,y,z,w} < \a min.
 * Returns \a max if \a val.{x,y,z,w} > \a max.
 * Otherwise, returns \a val.{x,y,z,w}.
 *
 * \param[in] val the vector to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val, with each component clamped between \a min and \a max.
 */
template <typename T, typename U>
constexpr auto clamp(const BasicVector4<T>& val, U min, U max) noexcept
{
    return BasicVector4<T>{clamp(val.x, min, max), clamp(val.y, min, max), clamp(val.z, min, max),
                           clamp(val.w, min, max)};
}

/**
 * \brief Clamps each component of a vector between 0 and 1
 *
 * Returns \a 0 if \a val.{x,y,z,w} < \a 0.
 * Returns \a 1 if \a val.{x,y,z,w} > \a 1.
 * Otherwise, returns \a val.{x,y,z,w}.
 *
 * \param[in] val the value to clamp
 *
 * \return \a val clamped between 0 and 1.
 */
template <typename T>
constexpr auto saturate(const BasicVector4<T>& val) noexcept
{
    return clamp(val, T{0}, T{1});
}

} // namespace khepri
