#include <khepri/scene/scene_object.hpp>

namespace khepri::scene {

const Behavior* SceneObject::behavior(std::type_index index) const noexcept
{
    const auto it = m_behaviors.find(index);
    return (it != m_behaviors.end()) ? it->second.get() : nullptr;
}

Behavior* SceneObject::behavior(std::type_index index) noexcept
{
    const auto it = m_behaviors.find(index);
    return (it != m_behaviors.end()) ? it->second.get() : nullptr;
}

Behavior& SceneObject::add_behavior(std::type_index index, std::unique_ptr<Behavior> behavior)
{
    return *(m_behaviors[index] = std::move(behavior));
}

bool SceneObject::remove_behavior(std::type_index index)
{
    return m_behaviors.erase(index) != 0;
}

std::any* SceneObject::user_data_typeless(std::type_index index)
{
    auto it = m_userdata.find(index);
    return (it != m_userdata.end()) ? &it->second : nullptr;
}

const std::any* SceneObject::user_data_typeless(std::type_index index) const
{
    const auto it = m_userdata.find(index);
    return (it != m_userdata.end()) ? &it->second : nullptr;
}

void SceneObject::user_data_typeless(std::type_index index, std::any data)
{
    m_userdata[index] = std::move(data);
}

} // namespace khepri::scene
