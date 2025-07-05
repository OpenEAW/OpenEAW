#pragma once

#include "color_rgb.hpp"
#include "math.hpp"

#include <cstdint>

namespace khepri {

/**
 * \brief An sRGB color
 *
 * This color is in \a sRGB space, so mathematical operations are not defined to avoid incorrect
 * results. sRGB colors can be displayed to the user, but must be converted to a \ref ColorRGB in
 * order to perform mathematical operations with it.
 *
 * Unlike \ref ColorRGB, this class stores its contents with an 8 bit integer per channel in
 * accordance with most sRGB output channels. This class is similar to \ref BasicVector3, except it
 * describes the semantics of its contents, and it does not provide any mathematical operations.
 *
 * \note This class does \a not clamp results after mathematical operations to the [0,1] range.
 */
#pragma pack(push, 1)
class ColorSRGB final
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

    /// Constructs an uninitialized ColorSRGB
    constexpr ColorSRGB() noexcept = default;

    /// Constructs the ColorSRGB from literals
    constexpr ColorSRGB(ComponentType fr, ComponentType fg, ComponentType fb) noexcept
        : r(fr), g(fg), b(fb)
    {
    }

    /**
     * Constructs a ColorSRGB from a color_rgb by performing linear-to-sRGB conversion.
     *
     * \note the components of the color are clamped to [0,1] before conversion.
     */
    explicit constexpr ColorSRGB(const ColorRGB& c) noexcept
        : r(static_cast<ComponentType>(linear_to_srgb(saturate(c.r)) *
                                       std::numeric_limits<ColorSRGB::ComponentType>::max()))
        , g(static_cast<ComponentType>(linear_to_srgb(saturate(c.g)) *
                                       std::numeric_limits<ColorSRGB::ComponentType>::max()))
        , b(static_cast<ComponentType>(linear_to_srgb(saturate(c.b)) *
                                       std::numeric_limits<ColorSRGB::ComponentType>::max()))
    {
    }

    /// Indexes the color. 0 is Red, 1 is Green, etc
    const ComponentType& operator[](int index) const noexcept
    {
        assert(index < 3);
        return gsl::span<const ComponentType>(&r, 3)[index];
    }

    /// Indexes the color. 0 is Red, 1 is Green, etc
    ComponentType& operator[](int index) noexcept
    {
        assert(index < 3);
        return gsl::span<ComponentType>(&r, 3)[index];
    }

    /**
     * Converts the value v from the linear color space to the sRGB color space.
     *
     * This is also known as gamma compression.
     */
    static constexpr float linear_to_srgb(float v) noexcept
    {
        // Conversion from CIE XYZ to sRGB
        // NOLINTNEXTLINE
        return v <= 0.0031308f ? 12.92f * v : (1.055f * pow(v, 1.0f / 2.4f) - 0.055f);
    }

    /**
     * Converts the value v from the sRGB color space to the linear color space
     *
     * This is also known as gamma expansion.
     */
    static constexpr float srgb_to_linear(float s) noexcept
    {
        // Conversion from sRGB to CIE XYZ
        // NOLINTNEXTLINE
        return s <= 0.04045f ? s / 12.92f : pow((s + 0.055f) / 1.055f, 2.4f);
    }
};
#pragma pack(pop)

constexpr ColorRGB::ColorRGB(const ColorSRGB& c) noexcept
    : r(ColorSRGB::srgb_to_linear(static_cast<float>(c.r) /
                                  std::numeric_limits<ColorSRGB::ComponentType>::max()))
    , g(ColorSRGB::srgb_to_linear(static_cast<float>(c.g) /
                                  std::numeric_limits<ColorSRGB::ComponentType>::max()))
    , b(ColorSRGB::srgb_to_linear(static_cast<float>(c.b) /
                                  std::numeric_limits<ColorSRGB::ComponentType>::max()))
{
}

} // namespace khepri
