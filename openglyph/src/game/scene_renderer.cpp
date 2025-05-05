#include <openglyph/game/behaviors/render_behavior.hpp>
#include <openglyph/game/scene_renderer.hpp>

namespace openglyph {
namespace {

class RenderState
{
public:
    struct Mesh
    {
        using Param = renderer::RenderModel::Mesh::Param;

        std::vector<Param> material_params;
    };

    explicit RenderState(const renderer::RenderModel& model, const khepri::Matrixf& transform)
        : meshes(model.meshes().size()), transform(transform)
    {
        const auto& model_meshes = model.meshes();
        for (std::size_t i = 0; i < model_meshes.size(); ++i) {
            meshes[i].material_params.insert(meshes[i].material_params.end(),
                                             model_meshes[i].material_params.begin(),
                                             model_meshes[i].material_params.end());
        }
    }

    std::vector<Mesh> meshes;
    khepri::Matrixf   transform;
};
} // namespace

void SceneRenderer::render_scene(const openglyph::Scene&         scene,
                                 const khepri::renderer::Camera& camera)
{
    std::vector<khepri::renderer::MeshInstance> meshes;

    for (const auto& object : scene.objects()) {
        if (const auto* render = object->behavior<RenderBehavior>()) {
            auto* state = object->user_data<RenderState>();
            if (!state) {
                object->user_data(
                    RenderState{render->model(), khepri::Matrixf::create_scaling(render->scale())});
                state = object->user_data<RenderState>();
            }

            const auto& transform    = object->transform();
            const auto& model_meshes = render->model().meshes();

            assert(model_meshes.size() == state->meshes.size());
            for (std::size_t i = 0; i < state->meshes.size(); ++i) {
                if (model_meshes[i].visible) {
                    meshes.push_back({model_meshes[i].render_mesh.get(),
                                      transform * state->transform, model_meshes[i].material,
                                      state->meshes[i].material_params});
                }
            }
        }
    }

    m_renderer.render_meshes(meshes, camera);
}

} // namespace openglyph
