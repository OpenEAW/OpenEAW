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

    /// Returns the skydome scenes
    [[nodiscard]] const auto& skydome_scenes() const noexcept
    {
        return m_skydome_scenes;
    }

    /// Returns the main scene
    [[nodiscard]] const auto& main_scene() const noexcept
    {
        return m_main_scene;
    }

    /**
     * Adds an object to the scene.
     *
     * Does nothing if the object is already added.
     */
    void add_object(const std::shared_ptr<khepri::scene::SceneObject>& object)
    {
        m_main_scene.add_object(object);
    }

    /**
     * Removes an object from the scene.
     *
     * Does nothing if the object is not in the scene.
     */
    void remove_object(const std::shared_ptr<khepri::scene::SceneObject>& object)
    {
        m_main_scene.remove_object(object);
    }

private:
    const GameObjectTypeStore& m_game_object_types;

    // The skydome scenes, these are rendererd behind all other objects and are not impacted by
    // z-near/far limitations of the camera.
    std::array<khepri::scene::Scene, Environment::NUM_SKYDOMES> m_skydome_scenes;

    // The main scene where all the action happens.
    khepri::scene::Scene m_main_scene;

    Environment m_environment;
};

} // namespace openglyph
