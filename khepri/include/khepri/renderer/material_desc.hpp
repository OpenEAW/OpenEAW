#pragma once

#include "render_pipeline_desc.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <khepri/math/matrix.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>
#include <khepri/math/vector4.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace khepri::renderer {

/**
 * \brief Description of a material
 *
 * A material is defined by a collection of shaders and a collection of related properties
 * (integers, floats, vectors, matrices and textures) that can be passed into a shader when
 * rendering a mesh.
 *
 * The properties in a material can be set by name in a #khepri::renderer::MeshInstance when passed
 * to the renderer. The specified (non-texture) properties are set in the shader's constant buffer
 * with name @c Material (if present). Texture properties are set on the shader's texture resource
 * with the same name directly.
 *
 * @c num_directional_lights and @c num_point_lights define the number of @c{DirectionalLight} and
 * @c{PointLight} objects, respectively, passed in their cbuffers to the shadersm filled with the
 * lighting information specified with \see{khepri::renderer::Renderer::set_dynamic_lights}.
 *
 * The shader's @c{EnvironmentConstants}' @c{NumDirectionalLights} and @c{NumPointLights} members
 * specify the actual number of lights that have been passed to the shader, but the shader is free
 * to ignore those fields since extra lights are inaccessible via the cbuffer anyway, and missing
 * lights are filled with 0-intensity black lights. So hardcoded calculations for the expected
 * number of lights should work regardless.
 */
struct MaterialDesc
{
    /// Value of a material shader property
    using PropertyValue =
        std::variant<std::int32_t, float, Vector2f, Vector3f, Vector4f, Matrixf, const Texture*>;

    /// Description of a material shader property
    struct Property
    {
        /// Property name
        std::string name;

        /// Default value of the property if none is provided by the mesh instance; this also
        /// determines the property's type
        PropertyValue default_value;
    };

    /// The type of the material. This is only used to allow render passes in the render pipeline to
    /// render certain materials. See #khepri::renderer::RenderPassDesc.
    std::string type;

    /// Number of directional lights the material's shader uses.
    int num_directional_lights{0};

    /// Number of point lights the material's shader uses.
    int num_point_lights{0};

    /// Shader of this material
    const Shader* shader{nullptr};

    /// Shader properties of this material
    std::vector<Property> properties;

    /**
     * Graphics pipeline options for this material.
     *
     * This field is used as an override on the options in the #RenderPipelineDesc. See
     * #GraphicsPipelineOptions for more information.
     */
    GraphicsPipelineOptions graphics_pipeline_options;
};

} // namespace khepri::renderer
