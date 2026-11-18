#pragma once

#include "faction.hpp"

namespace openglyph {

/**
 * @brief Represents a player.
 *
 * A player can be local, AI, or remote (via network).
 */
class Player final
{
public:
    explicit Player(const Faction& faction);

    /// Returns this player's faction
    const Faction& faction() const noexcept
    {
        return m_faction;
    }

private:
    const Faction& m_faction;
};

/**
 * Returns a functor that can be invoked with a `const Player&` and returns whether the player
 * belongs to the specified faction.
 */
[[nodiscard]] inline auto BelongsToFaction(const openglyph::Faction& faction) noexcept
{
    return [&](const openglyph::Player& player) { return &player.faction() == &faction; };
}

} // namespace openglyph
