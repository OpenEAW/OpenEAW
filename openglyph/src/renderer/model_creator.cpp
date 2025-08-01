#include <khepri/utility/string.hpp>

#include <openglyph/renderer/model_creator.hpp>

namespace openglyph::renderer {
namespace {
/**
 * Calculates the absolute transformation matrix for a bone in a model from the series of local
 * transformation in the bone hierarchy. This eventually creates a bone's absolute transformation in
 * the model (i.e. relative to the model's origin).
 */
khepri::Matrixf absolute_transform(const std::vector<Model::Bone>& bones,
                                   std::optional<std::uint32_t>    bone_index)
{
    khepri::Matrixf transform = khepri::Matrixf::IDENTITY;
    while (bone_index) {
        const auto& bone = bones[*bone_index];
        transform *= bone.parent_transform;
        bone_index = bone.parent_bone_index;
    }
    return transform;
}
} // namespace

ModelCreator::ModelCreator(khepri::renderer::Renderer&              renderer,
                           Loader<const khepri::renderer::Material> material_loader,
                           Loader<const khepri::renderer::Texture>  texture_loader)
    : m_renderer(renderer)
    , m_material_loader(std::move(material_loader))
    , m_texture_loader(std::move(texture_loader))
{
}

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
                if (const auto* const val = std::get_if<std::int32_t>(&param.value)) {
                    params.push_back({param.name, *val});
                } else if (const auto* const val = std::get_if<float>(&param.value)) {
                    params.push_back({param.name, *val});
                } else if (const auto* const val = std::get_if<khepri::Vector3f>(&param.value)) {
                    params.push_back({param.name, *val});
                } else if (const auto* const val = std::get_if<khepri::Vector4f>(&param.value)) {
                    params.push_back({param.name, *val});
                } else if (const auto* const val = std::get_if<std::string>(&param.value)) {
                    if (auto* texture = m_texture_loader(khepri::basename(*val))) {
                        params.push_back({param.name, texture});
                    }
                }
            }

            RenderModel::Mesh created_mesh{mesh.name,
                                           std::move(render_mesh),
                                           BillboardMode::none,
                                           render_material,
                                           std::move(params),
                                           mesh.visible,
                                           khepri::Matrixf::IDENTITY,
                                           khepri::Matrixf::IDENTITY};

            if (mesh.bone_index) {
                const auto& bone              = model.bones[*mesh.bone_index];
                created_mesh.billboard_mode   = bone.billboard_mode;
                created_mesh.root_transform   = absolute_transform(model.bones, mesh.bone_index);
                created_mesh.parent_transform = bone.parent_transform;
            }

            render_meshes.push_back(std::move(created_mesh));
        }
    }
    return std::make_unique<RenderModel>(std::move(render_meshes));
}

} // namespace openglyph::renderer
