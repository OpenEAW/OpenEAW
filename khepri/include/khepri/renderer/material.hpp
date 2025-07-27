#pragma once

#include "material_desc.hpp"

#include <cstddef>

namespace khepri::renderer {

/**
 * \brief A material.
 *
 * A material is a collection of shaders and a collection of related properties for rendering a
 * #khepri::renderer::Mesh. Materials are created by a #khepri::renderer::Renderer.
 *
 * \see #khepri::renderer::Renderer::create_material
 */
class Material
{
public:
    /// Parameter value
    using ParamValue = MaterialDesc::PropertyValue;

    /// Parameter description
    struct Param
    {
        /// Name of the parameter
        std::string name;
        /// Value of the parameter
        ParamValue value;
    };

    Material()          = default;
    virtual ~Material() = default;

protected:
    Material(const Material&)            = default;
    Material(Material&&)                 = default;
    Material& operator=(const Material&) = default;
    Material& operator=(Material&&)      = default;
};

} // namespace khepri::renderer
