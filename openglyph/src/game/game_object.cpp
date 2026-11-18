#include <openglyph/game/game_object.hpp>

namespace openglyph {

GameObject::GameObject(const openglyph::GameObjectType& type, const Player& owner)
    : m_type(type), m_owner(owner)
{
}

} // namespace openglyph
