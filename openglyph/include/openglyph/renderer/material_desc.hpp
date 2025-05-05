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

    using AlphaBlendMode  = khepri::renderer::MaterialDesc::AlphaBlendMode;
    using ComparisonFunc  = khepri::renderer::MaterialDesc::ComparisonFunc;
    using DepthBufferDesc = khepri::renderer::MaterialDesc::DepthBufferDesc;

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

    /// Type of alpha blending to use when rendering with this material
    AlphaBlendMode alpha_blend_mode{AlphaBlendMode::none};

    /// Depth-buffer settings to use when rendering this material
    std::optional<DepthBufferDesc> depth_buffer;

    /// Name of the shader of this material
    std::string shader;

    /// Shader properties of this material
    std::vector<Property> properties;
};

} // namespace openglyph::renderer
