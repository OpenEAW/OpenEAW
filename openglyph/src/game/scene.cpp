#include <openglyph/game/behaviors/render_behavior.hpp>
#include <openglyph/game/scene.hpp>
#include <openglyph/renderer/io/model.hpp>

namespace openglyph {

khepri::scene::Scene& Scene::target_scene(const khepri::scene::SceneObject& object)
{
    if (const auto* render = object.behavior<RenderBehavior>()) {
        switch (render->render_layer()) {
        case RenderBehavior::RenderLayer::background:
            return m_background_scene;
        case RenderBehavior::RenderLayer::foreground:
        default:
            break;
        }
    }
    return m_foreground_scene;
}

Scene::Scene(AssetCache& asset_cache, const GameObjectTypeStore& game_object_types,
             Environment environment)
    : m_game_object_types(game_object_types), m_environment(std::move(environment))
{
    // Create the skydomes
    for (auto i = 0; i < Environment::NUM_SKYDOMES; ++i) {
        const auto& skydome = m_environment.skydomes[i];
        if (const auto* type = m_game_object_types.get(skydome.name)) {
            auto object = std::make_shared<khepri::scene::SceneObject>();
            if (const auto* render_model = asset_cache.get_render_model(type->space_model_name)) {
                auto& behavior = object->create_behavior<openglyph::RenderBehavior>(*render_model);
                behavior.scale(type->scale_factor);
                if (type->is_in_background) {
                    behavior.render_layer(RenderBehavior::RenderLayer::background);
                }
            }
            object->scale({skydome.scale, skydome.scale, skydome.scale});
            object->rotation(khepri::Quaternion::from_euler(skydome.tilt, 0, skydome.z_angle,
                                                            khepri::ExtrinsicRotationOrder::zyx));
            add_object(std::move(object));
        }
    }

    // Set up the dynamic lighting
    m_dynamic_lights.directional_lights.reserve(m_environment.lights.size());
    for (const auto& light : m_environment.lights) {
        m_dynamic_lights.directional_lights.push_back(
            {-light.from_direction, 1.0, light.color, light.specular_color});
    }
}

void Scene::add_object(const std::shared_ptr<khepri::scene::SceneObject>& object)
{
    assert(object != nullptr);
    target_scene(*object).add_object(object);
}

void Scene::remove_object(const std::shared_ptr<khepri::scene::SceneObject>& object)
{
    // Try to remove from all scenes (to be safe)
    assert(object != nullptr);
    m_background_scene.remove_object(object);
    m_foreground_scene.remove_object(object);
}

Scene::~Scene() = default;

} // namespace openglyph
