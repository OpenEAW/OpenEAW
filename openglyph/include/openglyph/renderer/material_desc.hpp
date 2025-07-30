#pragma once

#include <khepri/renderer/material_desc.hpp>

#include <variant>

namespace openglyph::renderer {

/**
 * Description of a material
 */
struct MaterialDesc
{
    /// Value of a material shader property
    using PropertyValue = std::variant<std::int32_t, float, khepri::Vector2f, khepri::Vector3f,
                                       khepri::Vector4f, khepri::Matrixf, std::string>;

    /// Description of a material shader property
    struct Property
    {
        /// Property name
        std::string name;

        /// Default value of the property if none is provided by the mesh instance
        PropertyValue default_value;
    };

    /// Name of the material
    std::string name;

    /// The type of the material. This is only used to allow render passes in the render pipeline to
    /// render certain materials. See #khepri::renderer::RenderPassDesc.
    std::string type;

    /// Number of directional lights the material's shader uses.
    int num_directional_lights{0};

    /// Number of point lights the material's shader uses.
    int num_point_lights{0};

    /// Name of the shader of this material
    std::string shader;

    /// Shader properties of this material
    std::vector<Property> properties;
};

} // namespace openglyph::renderer
