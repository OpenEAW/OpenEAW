#pragma once

#include "behavior.hpp"

#include <khepri/math/matrix.hpp>
#include <khepri/math/quaternion.hpp>
#include <khepri/math/vector3.hpp>

#include <any>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace khepri::scene {

/**
 * Represents an object in a #khepri::scene::Scene.
 *
 * A @c SceneObject has positional information and a list of behaviors. The behaviors implement the
 * components of the entity-component-system.
 */
class SceneObject
{
public:
    SceneObject()          = default;
    virtual ~SceneObject() = default;

    SceneObject(const SceneObject&)            = delete;
    SceneObject(SceneObject&&)                 = delete;
    SceneObject& operator=(const SceneObject&) = delete;
    SceneObject& operator=(SceneObject&&)      = delete;

    /// Returns the position of the object in the scene
    const auto& position() const noexcept
    {
        return m_position;
    }

    /// Returns the scale modifier of the object
    const auto& scale() const noexcept
    {
        return m_scale;
    }

    /// Returns the rotation of the object in the scene
    const auto& rotation() const noexcept
    {
        return m_rotation;
    }

    /// Returns a transformation matrix for the object's position, scale and rotation.
    const auto& transform() const noexcept
    {
        return m_transform;
    }

    /// Sets the position of the object in the scene
    /// \param position the new position
    void position(const Vector3& position) noexcept
    {
        m_position = position;
        update_transform();
    }

    /// Sets the scale modifier the object
    /// \param scale the new scale modifier
    void scale(const Vector3& scale) noexcept
    {
        m_scale = scale;
        update_transform();
    }

    /// Sets the rotation of the object
    /// \param rotation the new rotation
    void rotation(const Quaternion& rotation) noexcept
    {
        m_rotation = rotation;
        update_transform();
    }

    /// Gets a behavior from the object
    /// \tparam Behavior the type of the behavior to retrieve
    /// \return the behavior, or null if the behavior does not exist
    template <typename Behavior>
    const Behavior* behavior() const noexcept
    {
        return static_cast<const Behavior*>(behavior(typeid(Behavior)));
    }

    /// Gets a behavior from the object
    /// \tparam Behavior the type of the behavior to retrieve
    /// \return the behavior, or null if the behavior does not exist
    template <typename Behavior>
    Behavior* behavior() noexcept
    {
        return static_cast<Behavior*>(behavior(typeid(Behavior)));
    }

    /// Creates and adds a behavior on the object
    /// \tparam Behavior the type of the behavior to create
    /// \param args the arguments to forward to the behavior's constructor
    /// \return the new behavior
    template <typename Behavior, typename... Args>
    Behavior& create_behavior(Args&&... args)
    {
        return static_cast<Behavior&>(add_behavior(
            typeid(Behavior), std::make_unique<Behavior>(std::forward<Args>(args)...)));
    }

    /// Removes a behavior from the object
    /// \tparam Behavior the type of the behavior to create
    /// \return true if the behavior was removed, false otherwise
    template <typename Behavior>
    bool remove_behavior()
    {
        return remove_behavior(typeid(Behavior));
    }

    /// Retrieves user data from the object.
    /// \tparam DataType the type of the user data to retrieve.
    /// \return the user data, or nullptr if no such user data was attached.
    template <typename DataType>
    const DataType* user_data() const
    {
        return std::any_cast<DataType>(user_data_typeless(typeid(DataType)));
    }

    /// Retrieves user data from the object.
    /// \tparam DataType the type of the user data to retrieve.
    /// \return the user data, or nullptr if no such user data was attached.
    template <typename DataType>
    DataType* user_data()
    {
        return std::any_cast<DataType>(user_data_typeless(typeid(DataType)));
    }

    /// Sets user data on the object
    /// \param data_type the user data to set on the object
    template <typename DataType>
    void user_data(DataType&& data_type)
    {
        user_data_typeless(typeid(DataType),
                           std::make_any<DataType>(std::forward<DataType>(data_type)));
    }

private:
    void update_transform() noexcept
    {
        m_transform =
            Matrixf::create_srt(Vector3f{m_scale}, Quaternionf{m_rotation}, Vector3f{m_position});
    }

    const class Behavior* behavior(std::type_index index) const noexcept;
    class Behavior*       behavior(std::type_index index) noexcept;
    class Behavior&       add_behavior(std::type_index index, std::unique_ptr<Behavior> behavior);
    bool                  remove_behavior(std::type_index index);

    std::any*       user_data_typeless(std::type_index index);
    const std::any* user_data_typeless(std::type_index index) const;
    void            user_data_typeless(std::type_index index, std::any data);

    Vector3    m_position{0, 0, 0};
    Vector3    m_scale{1, 1, 1};
    Quaternion m_rotation  = Quaternion::IDENTITY;
    Matrixf    m_transform = Matrixf::IDENTITY;

    std::unordered_map<std::type_index, std::unique_ptr<Behavior>> m_behaviors;

    mutable std::unordered_map<std::type_index, std::any> m_userdata;
};

} // namespace khepri::scene
