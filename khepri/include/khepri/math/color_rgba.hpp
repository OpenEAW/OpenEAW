#pragma once

#include "color_rgb.hpp"

namespace khepri {

/**
 * \brief An RGBA color
 *
 * This color is in \a linear space, so mathematical operations have the expected result.
 * However, it must be converted to a \ref ColorSRGB before displaying to a user.
 *
 * This class is similar to \ref BasicVector4, except it describes the semantics of its contents,
 * and it does not provide geometric operations such as \a length, \a dot, etc.
 *
 * \note This class does \a not clamp results after mathematical operations to the [0,1] range.
 */
#pragma pack(push, 1)
class ColorRGBA final
{
public:
    /// The type of the color's components
    using ComponentType = float;

    /// The red component of the color
    ComponentType r{};

    /// The green component of the color
    ComponentType g{};

    /// The blue component of the color
    ComponentType b{};

    /// The alpha component of the color
    ComponentType a{};

    /// Constructs an uninitialized ColorRGBA
    constexpr ColorRGBA() noexcept = default;

    /// Constructs the ColorRGBA from literals
    constexpr ColorRGBA(ComponentType fr, ComponentType fg, ComponentType fb,
                        ComponentType fa) noexcept
        : r(fr), g(fg), b(fb), a(fa)
    {}

    /// Constructs the ColorRGBA from a ColorRGB, and a float for Alpha
    constexpr ColorRGBA(const ColorRGB& c, ComponentType fa) noexcept
        : r(c.r), g(c.g), b(c.b), a(fa)
    {}

    /// Adds color \a c to the vector
    ColorRGBA& operator+=(const ColorRGBA& c) noexcept
    {
        r += c.r;
        g += c.g;
        b += c.b;
        a += c.a;
        return *this;
    }

    /// Subtracts color \a c from the color
    ColorRGBA& operator-=(const ColorRGBA& c) noexcept
    {
        r -= c.r;
        g -= c.g;
        b -= c.b;
        a -= c.a;
        return *this;
    }

    /// Scales the color by scalar \a s
    ColorRGBA& operator*=(ComponentType s) noexcept
    {
        r *= s;
        g *= s;
        b *= s;
        a *= s;
        return *this;
    }

    /// Scales the color with scalar 1 / \a s
    ColorRGBA& operator/=(ComponentType s) noexcept
    {
        r /= s;
        g /= s;
        b /= s;
        a /= s;
        return *this;
    }

    /// Indexes the color. 0 is Red, 1 is Green, etc
    const ComponentType& operator[](int index) const noexcept
    {
        assert(index < 4);
        return gsl::span<const ComponentType>(&r, 4)[index];
    }

    /// Indexes the color. 0 is Red, 1 is Green, etc
    ComponentType& operator[](int index) noexcept
    {
        assert(index < 4);
        return gsl::span<ComponentType>(&r, 4)[index];
    }
};
#pragma pack(pop)

/// Negates color \a c
inline ColorRGBA operator-(const ColorRGBA& c) noexcept
{
    return {-c.r, -c.g, -c.b, -c.a};
}

/// Adds color \a c2 to vector \a c1
inline ColorRGBA operator+(const ColorRGBA& c1, const ColorRGBA& c2) noexcept
{
    return {c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a};
}

/// Subtracts color \a c2 from color \a c1
inline ColorRGBA operator-(const ColorRGBA& c1, const ColorRGBA& c2) noexcept
{
    return {c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a};
}

/// Scales color \a c with scalar \a s
inline ColorRGBA operator*(const ColorRGBA& c, float s) noexcept
{
    return {c.r * s, c.g * s, c.b * s, c.a * s};
}

/// Scales color \a c with scalar \a s
inline ColorRGBA operator*(float s, const ColorRGBA& c) noexcept
{
    return {c.r * s, c.g * s, c.b * s, c.a * s};
}

/// Scales vector \a c with scalar 1/\a s
inline ColorRGBA operator/(const ColorRGBA& c, float s) noexcept
{
    return {c.r / s, c.g / s, c.b / s, c.a / s};
}

/// Modulates color \a c1 with color \a c2
inline ColorRGBA operator*(const ColorRGBA& c1, const ColorRGBA& c2) noexcept
{
    return {c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, c1.a * c2.a};
}

inline constexpr ColorRGB::ColorRGB(const ColorRGBA& c) noexcept : r(c.r), g(c.g), b(c.b) {}

/**
 * \brief Clamps each component of a color between two extremes
 *
 * Returns \a min if \a val.{r,g,b,a} < \a min.
 * Returns \a max if \a val.{r,g,b,a} > \a max.
 * Otherwise, returns \a val.{r,g,b,a}.
 *
 * \param[in] col the color to clamp
 * \param[in] min the lower boundary to clamp against
 * \param[in] max the upper boundary to clamp against
 *
 * \return \a val, with each component clamped between \a min and \a max.
 */
constexpr ColorRGBA clamp(const ColorRGBA& col, ColorRGBA::ComponentType min,
                          ColorRGBA::ComponentType max) noexcept
{
    return {clamp(col.r, min, max), clamp(col.g, min, max), clamp(col.b, min, max),
            clamp(col.a, min, max)};
}

} // namespace khepri
