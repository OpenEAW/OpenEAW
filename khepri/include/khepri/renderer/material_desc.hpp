#pragma once

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

    /// Shader of this material
    const Shader* shader{nullptr};

    /// Shader properties of this material
    std::vector<Property> properties;
};

} // namespace khepri::renderer
