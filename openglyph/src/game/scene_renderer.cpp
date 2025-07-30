#include <openglyph/game/behaviors/render_behavior.hpp>
#include <openglyph/game/scene_renderer.hpp>

namespace openglyph {
namespace {

// Objects, when rendered, need a rotation correction of 90 degrees around Z.
// Presumably, this is to to align -Y (the front in the model) with +X (the axis with no rotation,
// i.e. in-game "natural front").
// Weird, but this is the only way that aligns the behavior with the original game.
const auto OBJECT_ROTATION_CORRECTION = khepri::Matrixf::create_rotation(
    khepri::Quaternionf::from_axis_angle({0, 0, 1}, khepri::to_radians(90.0)));

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

/**
 * Overwrites \a transform's rotational aspects so that it aligns the -Y axis (typically "front" in
 * object space) with the \a front argument, and the +Z axis (typically "up" in object space) with
 * the \a up argument.
 */
void apply_billboard_transform(khepri::Matrixf& transform, khepri::Vector3f front,
                               khepri::Vector3f up)
{
    auto right = cross(up, front);
    up         = cross(front, right);

    front.normalize();
    right.normalize();
    up.normalize();

    // We want the +Y axis to align with `-front` vector so the -Y axis aligns with `front`.
    // The +Z axis is made to point in the same direction as the `up` vector.
    // Align the +X axis with the `right` vector, orthognal to `front` and `up`.
    // Also preserve the scale aspect of the transformation.
    const auto scale = transform.get_scale();
    transform.basis(right, -front, up);
    transform.pre_scale(scale);
}

/**
 * Overwrites \a transform's rotational aspect so that it aligns the -Y axis (typically "front" in
 * object space) with the \a front argument, but keeps the +Z axis (typically "up" in object space)
 * the same. In essence, this is a rotation around the +Z axis to align -Y with \a front when
 * projected onto the XY plane.
 */
void apply_z_billboard_transform(khepri::Matrixf& transform, khepri::Vector3f front)
{
    // Use the Z axis from the current transformation as "up"
    auto up = khepri::Vector3f(transform.col(2));

    // Create an orthogonal right
    auto right = cross(up, front);
    // Re-calculate front to be orthogonal to up and right, this will keep it in the local XY plane.
    // Afterwards, front will point away from the camera in the XY plane, so we can align +Y with
    // it.
    front = cross(up, right);

    front.normalize();
    right.normalize();
    up.normalize();

    // Also preserve the scale aspect of the transformation.
    const auto scale = transform.get_scale();
    transform.basis(right, front, up);
    transform.pre_scale(scale);
}

/**
 * Overwrites \a transform's rotational aspect so that it aligns the -Y axis (typically "front" in
 * object space) to face the camera, and the +Z axis (typically "up" in object space) with
 * the \a up argument.
 */
void apply_face_billboard_transform(khepri::Matrixf&                transform,
                                    const khepri::renderer::Camera& camera)
{
    const auto view_up = camera.matrices().view_inv.basis()[1];
    const auto obj_to_camera =
        khepri::Vector3f(normalize(camera.position() - transform.get_translation()));
    apply_billboard_transform(transform, obj_to_camera, view_up);
}

/**
 * Modifies the transformation matrix to apply the specified billboard mode.
 * This will overwrite the scale/rotation components of the transformation, and in some cases its
 * position too.
 */
void apply_billboard(khepri::Matrixf& transform, const renderer::RenderModel::Mesh& mesh,
                     const Environment& environment, const khepri::renderer::Camera& camera)
{
    // Note about billboarding:
    // The basis vectors of the inverse view matrix are the view space's X,Y,Z axes expressed in
    // world space. Note that because it's a right-handed view matrix, +Z points 'out of the
    // screen', towards the user, so it's the opposite of the view direction.
    switch (mesh.billboard_mode) {
    case renderer::BillboardMode::parallel: {
        const auto [_, view_up, view_neg_dir] = camera.matrices().view_inv.basis();
        // Turn the object's "front" to the camera's +Z, and its "up" to +Y.
        apply_billboard_transform(transform, view_neg_dir, view_up);
        break;
    }

    case renderer::BillboardMode::face:
        apply_face_billboard_transform(transform, camera);
        break;

    case renderer::BillboardMode::z_view: {
        // Rotate the object's front around local Z so it points parallel with the view direction.
        const auto view_neg_dir = camera.matrices().view_inv.basis()[2];
        apply_z_billboard_transform(transform, view_neg_dir);
        break;
    }

    case renderer::BillboardMode::z_wind:
        // Rotate the object's front around local Z so it points parallel with the wind.
        apply_z_billboard_transform(transform, khepri::Vector3f(environment.wind.to_direction, 0));
        break;

    case renderer::BillboardMode::z_light:
        // Rotate the object's front around local Z so it points parallel with the main light.
        apply_z_billboard_transform(transform, -environment.lights[0].from_direction);
        break;

    case renderer::BillboardMode::sun: {
        // Position the object fixed w.r.t the camera, towards the main light
        const auto distance = mesh.parent_transform.get_translation().length();
        transform.set_translation(khepri::Vector3f(camera.position()) +
                                  environment.lights[0].from_direction * distance);
        // Also make it face the camera
        apply_face_billboard_transform(transform, camera);
        break;
    }

    case renderer::BillboardMode::sun_glow: {
        // Get offset from the parent bone (maintain scaling of final transformation)
        const auto offset_from_parent =
            mesh.parent_transform.get_translation() * transform.get_rotation_scale();
        const auto distance = offset_from_parent.length();
        // Rotate the object around its parent towards the main light
        transform.set_translation(transform.get_translation() - offset_from_parent +
                                  environment.lights[0].from_direction * distance);
        // Also make it face the camera
        apply_face_billboard_transform(transform, camera);
        break;
    }

    case renderer::BillboardMode::none:
    default:
        break;
    }
}
} // namespace

void SceneRenderer::render_scene(const openglyph::Scene&         scene,
                                 const khepri::renderer::Camera& camera)
{
    // Set the lights
    m_renderer.set_dynamic_lights(scene.dynamic_lights());

    // Create a copy of the camera for rendering the background scene. This camera has a larger
    // znear and zfar to ensure that the background objects are visible from all distances.
    khepri::renderer::Camera background_camera = camera;
    background_camera.znear(10.0f);
    background_camera.zfar(100000.0f);

    render_scene(scene.background_scene(), scene.environment(), background_camera);

    // Clear the depth and stencil buffers after rendering the background scene so that they can
    // properly layer without Z-fighting.
    m_renderer.clear(khepri::renderer::Renderer::clear_depth |
                     khepri::renderer::Renderer::clear_stencil);

    // Use the normal, provided camera to render the main scene.
    render_scene(scene.foreground_scene(), scene.environment(), camera);
}

void SceneRenderer::render_scene(const khepri::scene::Scene&     scene,
                                 const openglyph::Environment&   environment,
                                 const khepri::renderer::Camera& camera)
{
    std::vector<khepri::renderer::MeshInstance> meshes;

    for (const auto& object : scene.objects()) {
        if (const auto* render = object->behavior<RenderBehavior>()) {
            auto* state = object->user_data<RenderState>();
            if (state == nullptr) {
                object->user_data(RenderState{
                    render->model(),
                    khepri::Matrixf::create_scaling(static_cast<float>(render->scale()))});
                state = object->user_data<RenderState>();
            }

            const auto& scene_transform = OBJECT_ROTATION_CORRECTION * object->transform();
            const auto& model_meshes    = render->model().meshes();

            assert(model_meshes.size() == state->meshes.size());
            for (std::size_t i = 0; i < state->meshes.size(); ++i) {
                if (model_meshes[i].visible) {
                    // Create the mesh's transformation: first transform the mesh according to the
                    // in-model's transformation. Then apply any object-specific transformations
                    // (first scale from the RenderState, then the rotation and position in the
                    // scene).
                    khepri::Matrixf transform =
                        model_meshes[i].root_transform * state->transform * scene_transform;

                    if (model_meshes[i].billboard_mode != renderer::BillboardMode::none) {
                        //  Then apply billboarding. This will overwrite the rotation
                        //  components of the transformation, and in some cases its position too.
                        apply_billboard(transform, model_meshes[i], environment, camera);
                    }

                    meshes.push_back({model_meshes[i].render_mesh.get(), transform,
                                      model_meshes[i].material, state->meshes[i].material_params});
                }
            }
        }
    }

    m_renderer.render_meshes(m_render_pipeline, meshes, camera);
}

} // namespace openglyph
