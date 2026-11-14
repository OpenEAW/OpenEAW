#pragma once

#include "material.hpp"
#include "mesh.hpp"

#include <khepri/math/matrix.hpp>

#include <span>

namespace khepri::renderer {

/**
 * \brief An instance of a mesh
 *
 * A mesh is just a template for the renderer. It can be instantiated multiple times with
 * this type. A #khepri::renderer::Renderer renders these instances.
 *
 * \see #khepri::renderer::Renderer::render_meshes
 */
struct MeshInstance
{
    /// The mesh this is an instance of
    const Mesh* mesh{nullptr};

    /// The transformation matrix for this instance
    Matrixf transform;

    /// The material to render this instance with
    const Material* material{nullptr};

    /// Material parameters for this instance
    std::span<Material::Param> material_params;
};

} // namespace khepri::renderer
