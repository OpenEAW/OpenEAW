#pragma once

#include <cstdint>

namespace openglyph::renderer {

/**
 * \brief Billboard modes for bones.
 *
 * This enum describes how objects attached to a bone should be rotated to face e.g. the camera, the
 * main light source, the wind, etc.
 */
enum class BillboardMode : std::uint8_t
{
    /// No billboard mode, objects are untouched.
    none,

    /// Rotate object around its local origin so the "front" axis is parallel with the camera's
    /// direction.
    parallel,

    /// Rotate object around its local origin so the "front" axis points to the camera.
    face,

    /// Rotate object around its local Z axis so the "front" axis is parallel with the camera's
    /// direction.
    z_view,

    ///< Rotate object around its local Z axis so the "front" axis points in the wind direction.
    z_wind,

    /// Rotate object around its local Z axis so the "front" axis points to the main light
    /// source.
    z_light,

    /// Rotate object around its parent's origin so that the +X axis points to the main light in
    /// view space, but the object remains in a plane parallel to the camera in world space. Also
    /// rotates the object as if \a View billboard mode was set.
    sun_glow,

    /// Rotate object around its parent's origin so that the +X axis points to the main light. Also
    /// rotates the object as if \a View billboard mode was set.
    sun,
};

} // namespace openglyph::renderer
