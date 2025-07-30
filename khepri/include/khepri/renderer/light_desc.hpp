#pragma once

#include <khepri/math/color_rgb.hpp>
#include <khepri/math/vector3.hpp>

#include <vector>

namespace khepri::renderer {

/**
 * Describes a directional light.
 *
 * Directional lights are infinitely far away and have all rays in parallel.
 * This is computationally the easiest light source to compute lighting for.
 */
struct DirectionalLightDesc
{
    /// Direction the light is pointing to (i.e. the direction the rays travel), in world space.
    Vector3 direction{0, 0, -1};

    /**
     * Intensity of the light.
     *
     * This is effectively a multiplier for the colors.
     */
    double intensity{0.0};

    /// Light's color for diffuse reflection.
    ColorRGB diffuse_color{0, 0, 0};

    /// Light's color for specular reflection.
    ColorRGB specular_color{0, 0, 0};
};

/**
 * Describes a point light.
 *
 * Point lights describe a light source located at a specific point, radiating outward in all
 * directions around it. They are computationally more expensive that directional lights.
 *
 * Note that the light intensity passed to the shader is the original intensity specified here, so
 * the shader can calculate accurate per-vertex dropoff.
 */
struct PointLightDesc
{
    /// Position of the light, in world space.
    Vector3 position{0, 0, 0};

    /// Intensity of the light.
    double intensity{0.0};

    /// Light's color for diffuse reflection.
    ColorRGB diffuse_color{0, 0, 0};

    /// Light's color for specular reflection.
    ColorRGB specular_color{0, 0, 0};

    /**
     * Maximum distance the light reaches.
     * Light intensity drops off by the inverse-square law.
     */
    double max_distance{0};
};

/**
 * Describes the dynamic light sources for a scene.
 *
 * This information is passed to shaders, which use it in lighting calculations.
 *
 * Note that these lights are meant to be used for direct lighting calculation shaders. Static
 * lights which are e.g. baked into textures do not need to be specified here.
 *
 * Note that shaders are authored to accept up to a fixed number of certain types of lights. Extra
 * lights specified here are ignored. Missing lights are filled with default-initialized lights
 * (black).
 */
struct DynamicLightDesc
{
    /**
     * Directional lights.
     *
     * See \see DirectionalLightDesc for more info on directional lights.
     *
     * Like all dynamic lights, these are passed to shaders, but directional lights are prioritized
     * according to their order. If a shader accepts N directional lights, the first N in this
     * vector will be passed.
     */
    std::vector<DirectionalLightDesc> directional_lights;

    /**
     * Point lights.
     *
     * See \see PointLightDesc for more info on point lights.
     *
     * Like all dynamic lights, these are passed to shaders, but point lights are prioritized
     * according to their intensity at the rendered object's position: if a shader for an object
     * accepts N point lights, the renderer finds the top N point lights (if any) that are the most
     * luminous for an object's position, based on the light's intensity and distance drop-off, and
     * passes them to the shader, in order of decreasing luminosity.
     */
    std::vector<PointLightDesc> point_lights;
};

} // namespace khepri::renderer
