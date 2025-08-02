#pragma once

#include "color_rgb.hpp"
#include "color_srgb.hpp"
#include "math.hpp"

#include <cstdint>

namespace khepri {

/**
 * \brief An sRGB+A color
 *
 * This color is in \a sRGB space, so mathematical operations are not defined to avoid incorrect
 * results. The Alpha component is in linear space as it is a fraction, not a color.
 * sRGB colors can be displayed to the user, but must be converted to a \ref ColorRGBA in order to
 * perform mathematical operations with it.
 *
 * Unlike \ref ColorRGBA, this class stores its contents with an 8 bit integer per channel in
 * accordance with most sRGB+A output channels. This class is similar to \ref BasicVector4, except
 * it describes the semantics of its contents, and it does not provide any mathematical operations.
 *
 * \note This class does \a not clamp results after mathematical operations to the [0,1] range.
 */
#pragma pack(push, 1)
class ColorSRGBA final
{
public:
    /// The type of the color's components
    using ComponentType = std::uint8_t;

    /// The red component of the color
    ComponentType r{};

    /// The green component of the color
    ComponentType g{};

    /// The blue component of the color
    ComponentType b{};

    /// The alpha component of the color
    ComponentType a{};

    /// Constructs an uninitialized ColorSRGBA
    constexpr ColorSRGBA() noexcept = default;

    /// Constructs the ColorSRGBA from literals
    constexpr ColorSRGBA(ComponentType fr, ComponentType fg, ComponentType fb,
                         ComponentType fa) noexcept
        : r(fr), g(fg), b(fb), a(fa)
    {
    }

    /// Constructs the ColorSRGBA from a ColorSRGB, and an Alpha component
    constexpr ColorSRGBA(const ColorSRGB& c, ComponentType fa) noexcept
        : r(c.r), g(c.g), b(c.b), a(fa)
    {
    }

    /**
     * Constructs a ColorSRGBA from a ColorRGBA by performing linear-to-sRGB conversion (except for
     * the alpha component).
     *
     * \note the components of the color are clamped to [0,1] before conversion.
     */
    explicit constexpr ColorSRGBA(const ColorRGBA& c) noexcept
        : r(static_cast<ComponentType>(ColorSRGB::linear_to_srgb(saturate(c.r)) *
                                       std::numeric_limits<ColorSRGBA::ComponentType>::max()))
        , g(static_cast<ComponentType>(ColorSRGB::linear_to_srgb(saturate(c.g)) *
                                       std::numeric_limits<ColorSRGBA::ComponentType>::max()))
        , b(static_cast<ComponentType>(ColorSRGB::linear_to_srgb(saturate(c.b)) *
                                       std::numeric_limits<ColorSRGBA::ComponentType>::max()))
        , a(static_cast<ComponentType>(saturate(c.a) *
                                       std::numeric_limits<ColorSRGBA::ComponentType>::max()))
    {
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

constexpr ColorRGBA::ColorRGBA(const ColorSRGBA& c) noexcept
    : r(ColorSRGB::srgb_to_linear(static_cast<float>(c.r) /
                                  std::numeric_limits<ColorSRGBA::ComponentType>::max()))
    , g(ColorSRGB::srgb_to_linear(static_cast<float>(c.g) /
                                  std::numeric_limits<ColorSRGBA::ComponentType>::max()))
    , b(ColorSRGB::srgb_to_linear(static_cast<float>(c.b) /
                                  std::numeric_limits<ColorSRGBA::ComponentType>::max()))
    , a(static_cast<float>(c.a) / std::numeric_limits<ColorSRGBA::ComponentType>::max())
{
}

} // namespace khepri
