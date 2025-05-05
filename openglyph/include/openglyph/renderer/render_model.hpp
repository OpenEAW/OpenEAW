#pragma once

#include <khepri/renderer/material.hpp>
#include <khepri/renderer/mesh.hpp>
#include <khepri/renderer/mesh_instance.hpp>

#include <vector>

namespace openglyph::renderer {

class RenderModel
{
public:
    struct Mesh
    {
        using Param = khepri::renderer::Material::Param;

        std::string                             name;
        std::unique_ptr<khepri::renderer::Mesh> render_mesh;
        khepri::renderer::Material*             material;
        std::vector<Param>                      material_params;
        bool                                    visible;
    };

    explicit RenderModel(std::vector<Mesh> meshes) : m_meshes(std::move(meshes)) {}

    const auto& meshes() const noexcept
    {
        return m_meshes;
    }

private:
    std::vector<Mesh> m_meshes;
};

} // namespace openglyph::renderer
