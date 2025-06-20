#pragma once

#include <cstddef>

namespace khepri::renderer {

/**
 * \brief A mesh.
 *
 * A mesh is a collection of geometry that can be rendered. Meshes are created by a
 * #khepri::renderer::Renderer.
 *
 * \see #khepri::renderer::Renderer::create_mesh
 */
class Mesh
{
public:
    Mesh()          = default;
    virtual ~Mesh() = default;

    Mesh(const Mesh&)            = delete;
    Mesh(Mesh&&)                 = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&)      = delete;
};

} // namespace khepri::renderer
