#pragma once

#include "environment.hpp"
#include "game_object_type_store.hpp"

#include <khepri/scene/scene.hpp>
#include <khepri/utility/cache.hpp>

#include <openglyph/assets/asset_cache.hpp>

#include <vector>

namespace openglyph {

/**
 * \brief A scene
 *
 * A scene is a collection of scene objects along with environmental properties and represents an
 * interactive space.
 */
class Scene
{
public:
    Scene(AssetCache& asset_cache, const GameObjectTypeStore& game_object_types,
          Environment environment);

    Scene(const Scene&)                = delete;
    Scene(Scene&&) noexcept            = delete;
    Scene& operator=(const Scene&)     = delete;
    Scene& operator=(Scene&&) noexcept = delete;
    ~Scene();

    /// Returns a reference to the environment for this scene
    [[nodiscard]] const Environment& environment() const noexcept
    {
        return m_environment;
    }

    /// Returns the background scene
    [[nodiscard]] const auto& background_scene() const noexcept
    {
        return m_background_scene;
    }

    /// Returns the main scene
    [[nodiscard]] const auto& foreground_scene() const noexcept
    {
        return m_foreground_scene;
    }

    /// Returns the dynamic lights of the scene
    [[nodiscard]] const auto& dynamic_lights() const noexcept
    {
        return m_dynamic_lights;
    }

    /**
     * Adds an object to the scene.
     *
     * Does nothing if the object is already added.
     */
    void add_object(const std::shared_ptr<khepri::scene::SceneObject>& object);

    /**
     * Removes an object from the scene.
     *
     * Does nothing if the object is not in the scene.
     */
    void remove_object(const std::shared_ptr<khepri::scene::SceneObject>& object);

    template <typename BehaviorType>
    std::vector<std::shared_ptr<khepri::scene::SceneObject>> objects() const
    {
        // Combine the objects from the foreground and background scenes
        auto result    = m_foreground_scene.objects<BehaviorType>();
        auto bg_result = m_background_scene.objects<BehaviorType>();
        result.insert(result.end(), bg_result.begin(), bg_result.end());
        return result;
    }

private:
    // Returns a reference to the scene the object should be placed into
    khepri::scene::Scene& target_scene(const khepri::scene::SceneObject& object);

    const GameObjectTypeStore& m_game_object_types;

    // Special "background" scene that's rendered behind the foreground scene with special depth
    // range and no fog. Useful for objects that should always appear in the background.
    khepri::scene::Scene m_background_scene;

    // The main foreground scene where all the action happens.
    khepri::scene::Scene m_foreground_scene;

    // The dynamic lighting setup for this scene
    khepri::renderer::DynamicLightDesc m_dynamic_lights;

    Environment m_environment;
};

} // namespace openglyph
