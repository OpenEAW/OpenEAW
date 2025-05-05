#pragma once

#include <array>
#include <string>

namespace openglyph {

/**
 * @brief Skydome properties
 */
struct Skydome
{
    /// Name of the skydome's game object
    std::string name;

    /// Scale to instantiate the skydome with
    double scale{1.0};

    // Rotation around the X-axis (in radians) to instantiate the skydome with
    double tilt{0};

    // Rotation around the Z-axis (in radians) to instantiate the skydome with
    double z_angle{0};
};

/**
 * @brief Environment properties
 *
 * An environment describes the physical characteristics of a scene. Lighting, backdrop, weather,
 * and so on.
 */
struct Environment
{
    /**
     * @brief The environment's name.
     */
    std::string name;

    /**
     * @brief The environment's skydomes.
     *
     * Skydomes are rendered behind all other objects and typically provide a full 360-degree view
     * of some environment. The skydomes should be rendered on top of each other, in the order they
     * are stored. This allows all but the first skydome to be translucent for complex combinations.
     */
    std::array<Skydome, 2> skydomes;
};

} // namespace openglyph