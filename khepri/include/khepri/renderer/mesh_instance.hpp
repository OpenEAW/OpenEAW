#pragma once

#include "material.hpp"
#include "material_desc.hpp"
#include "mesh.hpp"
#include "texture.hpp"

#include <khepri/math/matrix.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>
#include <khepri/math/vector4.hpp>

#include <gsl/gsl-lite.hpp>

#include <cstdint>
#include <string>
#include <variant>

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
    gsl::span<Material::Param> material_params;
};

} // namespace khepri::renderer
