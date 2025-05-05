#include <openglyph/game/behaviors/render_behavior.hpp>
#include <openglyph/game/scene.hpp>
#include <openglyph/renderer/io/model.hpp>

namespace openglyph {

Scene::Scene(AssetCache& asset_cache, const GameObjectTypeStore& game_object_types,
             Environment environment)
    : m_game_object_types(game_object_types), m_environment(std::move(environment))
{
    for (const auto& skydome : m_environment.skydomes) {
        if (auto* type = m_game_object_types.get(skydome.name)) {
            auto object = std::make_shared<khepri::scene::SceneObject>();
            if (const auto* render_model = asset_cache.get_render_model(type->space_model_name)) {
                auto& behavior = object->create_behavior<openglyph::RenderBehavior>(*render_model);
                behavior.scale(type->scale_factor);
            }
            object->scale({skydome.scale, skydome.scale, skydome.scale});
            object->rotation(khepri::Quaternion::from_euler(skydome.tilt, 0, skydome.z_angle));
            add_object(object);
        }
    }
}

Scene::~Scene() = default;

} // namespace openglyph
