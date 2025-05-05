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

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    ~Scene();

    /// Returns the objects in the scene
    [[nodiscard]] const auto& objects() const noexcept
    {
        return m_scene.objects();
    }

    /**
     * Adds an object to the scene.
     *
     * Does nothing if the object is already added.
     */
    void add_object(const std::shared_ptr<khepri::scene::SceneObject>& object)
    {
        m_scene.add_object(object);
    }

    /**
     * Removes an object from the scene.
     *
     * Does nothing if the object is not in the scene.
     */
    void remove_object(const std::shared_ptr<khepri::scene::SceneObject>& object)
    {
        m_scene.remove_object(object);
    }

private:
    const GameObjectTypeStore& m_game_object_types;

    khepri::scene::Scene m_scene;

    Environment m_environment;
};

} // namespace openglyph
