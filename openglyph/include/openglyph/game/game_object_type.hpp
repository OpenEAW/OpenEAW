#pragma once

#include <string>

namespace openglyph {

/**
 * @brief Describes a GameObject
 *
 * Contains all properties to describe a single game object.
 *
 * @note a GameObjectType is a read-only *view* into the data. The data is typically owned by the
 * class loading and managing these objects.
 */
struct GameObjectType final
{
    /// The name of this game object type
    std::string_view name;

    /// The name of the render model to use in space mode
    std::string_view space_model_name;

    /// The factor by which the model be scaled
    double scale_factor{1.0};

    /// Should this object be rendered in the background layer?
    bool is_in_background;

    /// Is this type a marker?
    bool is_marker{false};
};

} // namespace openglyph
