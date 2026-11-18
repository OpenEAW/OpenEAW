#pragma once

#include <string>

namespace openglyph {

/**
 * @brief Represents a faction in the game
 *
 * This class represents a single faction and holds all of its data.
 */
struct Faction
{
    /// The (internal) ID of the faction
    std::string id;

    /// The ID of the localized human-readable string with the faction's name.
    std::string display_name_id;

    /// Whether or not this faction is playable by a player.
    bool is_playable{false};
};

} // namespace openglyph