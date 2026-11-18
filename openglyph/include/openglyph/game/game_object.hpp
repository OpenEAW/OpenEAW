#pragma once

#include "game_object_type.hpp"
#include "player.hpp"

#include <khepri/scene/scene_object.hpp>

namespace openglyph {

class GameObject : public khepri::scene::SceneObject
{
public:
    GameObject(const GameObjectType& type, const Player& owner);

    [[nodiscard]] const GameObjectType& type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const Player& owner() const noexcept
    {
        return m_owner;
    }

private:
    const openglyph::GameObjectType& m_type;
    const Player&                    m_owner;
};

} // namespace openglyph