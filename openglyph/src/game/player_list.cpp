#include <khepri/exceptions.hpp>

#include <openglyph/game/player_list.hpp>

#include <unordered_set>

namespace openglyph {

PlayerList::PlayerList(std::vector<Player> players, std::size_t local_player_index)
    : m_players(std::move(players)), m_local_player_index(local_player_index)
{
}

PlayerList::Builder::Builder(const FactionList& factions) : m_factions(factions) {}

void PlayerList::Builder::add_player(Player&& player)
{
    m_players.push_back(std::move(player));
}

void PlayerList::Builder::set_local_player(std::size_t index)
{
    if (index >= m_players.size()) {
        throw khepri::ArgumentError();
    }
    m_local_player_index = index;
}

PlayerList PlayerList::Builder::build() &&
{
    if (!m_local_player_index) {
        throw khepri::InvalidOperationException();
    }

    std::unordered_set<const Faction*> seen_factions;
    for (const auto& player : m_players) {
        seen_factions.insert(&player.faction());
    }

    // Create a dummy player for factions without a real player.
    // This way, all objects can be assigned an owner.
    for (const auto& faction : m_factions.all()) {
        if (!seen_factions.contains(&faction)) {
            m_players.emplace_back(faction);
        }
    }

    return PlayerList(std::move(m_players), *m_local_player_index);
}

} // namespace openglyph