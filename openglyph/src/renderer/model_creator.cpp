#include <khepri/utility/string.hpp>
#include <openglyph/renderer/model_creator.hpp>

namespace openglyph::renderer {

ModelCreator::ModelCreator(khepri::renderer::Renderer&        renderer,
                           Loader<khepri::renderer::Material> material_loader,
                           Loader<khepri::renderer::Texture>  texture_loader)
    : m_renderer(renderer)
    , m_material_loader(std::move(material_loader))
    , m_texture_loader(std::move(texture_loader))
{}

std::unique_ptr<RenderModel> ModelCreator::create_model(const Model& model)
{
    std::vector<RenderModel::Mesh> render_meshes;
    for (const auto& mesh : model.meshes) {
        const auto& material = mesh.materials[0];
        if (auto* render_material = m_material_loader(khepri::basename(material.name))) {
            // Create the renderable mesh
            khepri::renderer::MeshDesc mesh_desc;
            mesh_desc.vertices.reserve(material.vertices.size());
            for (const auto& v : material.vertices) {
                mesh_desc.vertices.push_back(
                    {v.position, v.normal, v.tangent, v.binormal, v.uv[0], v.color});
            }
            mesh_desc.indices = material.indices;
            auto render_mesh  = m_renderer.create_mesh(mesh_desc);

            // Set up material parameters
            std::vector<RenderModel::Mesh::Param> params;
            for (const auto& param : material.params) {
                if (auto val = std::get_if<std::int32_t>(&param.value)) {
                    params.push_back({param.name, *val});
                } else if (auto val = std::get_if<float>(&param.value)) {
                    params.push_back({param.name, *val});
                } else if (auto val = std::get_if<khepri::Vector3f>(&param.value)) {
                    params.push_back({param.name, *val});
                } else if (auto val = std::get_if<khepri::Vector4f>(&param.value)) {
                    params.push_back({param.name, *val});
                } else if (auto val = std::get_if<std::string>(&param.value)) {
                    if (auto* texture = m_texture_loader(khepri::basename(*val))) {
                        params.push_back({param.name, texture});
                    }
                }
            }

            render_meshes.push_back({mesh.name, std::move(render_mesh), render_material,
                                     std::move(params), mesh.visible});
        }
    }
    return std::make_unique<RenderModel>(std::move(render_meshes));
}

} // namespace openglyph::renderer
