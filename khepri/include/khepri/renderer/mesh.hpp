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

protected:
    Mesh(const Mesh&)            = default;
    Mesh(Mesh&&)                 = default;
    Mesh& operator=(const Mesh&) = default;
    Mesh& operator=(Mesh&&)      = default;
};

} // namespace khepri::renderer
