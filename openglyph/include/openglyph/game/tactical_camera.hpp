#pragma once

#include <khepri/math/interpolator.hpp>

#include <variant>

namespace openglyph::renderer {

/**
 * @brief Describes a tactical camera
 */
struct TacticalCamera
{
    /// Describes a simple range
    struct Range
    {
        /// Lower bound of the range (inclusive)
        double min;

        /// Upper bound of the range (inclusive)
        double max;

        /// Default value for the range
        double default_value;
    };

    /// Describes a camera property
    struct Property
    {
        // The range of values for the property. This can either be a simple range which can be
        // arbitrarily controlled by the user, or an interpolator which ties the property to the
        // camera's current zoom factor ([0,1]).
        std::variant<Range, khepri::CubicInterpolator> range;

        /// The number of input "steps" the range covers.
        /// The larger this number, the slower traversing the range will be for a given input.
        unsigned int steps;
    };

    /// The camera's name
    std::string name;

    /// The camera's pitch, in radians.
    Property pitch;

    /// The camera's distance-from-target, in world units.
    Property distance;

    /// The camera's yaw, in radians.
    Property yaw;

    /// The camera's field of view, in radians.
    Property fov;

    /// Near clip plane distance, in world units in front of the camera position
    double near_clip{10.0};

    /// Far clip plane distance, in world units in front of the camera position
    double far_clip{10000.0};
};

} // namespace openglyph::renderer
