#pragma once

#include <khepri/math/color_rgb.hpp>
#include <khepri/math/vector3.hpp>

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

struct Light
{
    /// Direction the light is pointing **from**
    khepri::Vector3f from_direction{0, 1, 0};

    /// Diffuse color of the light
    khepri::ColorRGB color{1, 1, 1};

    /// Specular color of the light
    khepri::ColorRGB specular_color{0, 0, 0};

    // Intensity of the light
    float intensity{0.5};
};

struct Wind
{
    /// Direction of the window on the XY plane
    khepri::Vector2f to_direction;

    // Speed of the wind, in world units per seconds
    float speed;
};

/**
 * @brief Environment properties
 *
 * An environment describes the physical characteristics of a scene. Lighting, backdrop, weather,
 * and so on.
 */
struct Environment
{
    static constexpr auto NUM_SKYDOMES = 2;
    static constexpr auto NUM_LIGHTS   = 3;

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
    std::array<Skydome, NUM_SKYDOMES> skydomes;

    /**
     * @brief The lights in the scene.
     *
     * A scene typically has multiple light. The first light (i.e. `lights[0]`) is always the 'main'
     * light that casts shadows, creates specular effects, and so on.
     */
    std::array<Light, NUM_LIGHTS> lights;

    /**
     * @brief Ambient color of the scene. All objects in the scene are always lit by an
     * omnidirectional light with this color, even if all \a lights are off.
     */
    khepri::ColorRGB ambient_color{0.1, 0.1, 0.1};

    /**
     * @brief Wind information of the scene. Wind is mostly a visual effect that impacts particles,
     * clouds, and some mesh orientations (e.g. flags).
     */
    Wind wind;
};

} // namespace openglyph