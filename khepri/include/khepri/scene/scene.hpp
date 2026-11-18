#pragma once

#include "scene_object.hpp"

#include <memory>
#include <set>

namespace khepri::scene {

/**
 * \brief A scene
 *
 * A scene is a collection of scene objects and represents an interactive space.
 */
class Scene
{
public:
    Scene()          = default;
    virtual ~Scene() = default;

    Scene(const Scene&)                = delete;
    Scene(Scene&&) noexcept            = delete;
    Scene& operator=(const Scene&)     = delete;
    Scene& operator=(Scene&&) noexcept = delete;

    /// Returns the objects in the scene
    [[nodiscard]] const auto& objects() const noexcept
    {
        return m_objects;
    }

    /**
     * Adds an object to the scene.
     *
     * Does nothing if the object is already added.
     */
    void add_object(const std::shared_ptr<SceneObject>& object)
    {
        m_objects.insert(object);
    }

    /**
     * Removes an object from the scene.
     *
     * Does nothing if the object is not in the scene.
     */
    void remove_object(const std::shared_ptr<SceneObject>& object)
    {
        m_objects.erase(object);
    }

    /**
     * Returns all objects in the scene that have a specified behavior.
     * \tparam BehaviorType the behavior that must be present on returned objects.
     * \tparam ObjectType return the objects as this type (if they are of that derived type).
     */
    template <typename BehaviorType, typename ObjectType = khepri::scene::SceneObject>
        requires std::derived_from<ObjectType, khepri::scene::SceneObject>
    std::vector<std::shared_ptr<ObjectType>> objects() const
    {
        std::vector<std::shared_ptr<ObjectType>> result;
        for (const auto& object : m_objects) {
            if (auto x = std::dynamic_pointer_cast<ObjectType>(object);
                x && object->behavior<BehaviorType>()) {
                result.push_back(std::move(x));
            }
        }
        return result;
    }

private:
    std::set<std::shared_ptr<SceneObject>> m_objects;
};

} // namespace khepri::scene
