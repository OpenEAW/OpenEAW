#pragma once

#include "faction_list.hpp"
#include "player.hpp"

#include <cassert>
#include <optional>
#include <ranges>
#include <vector>

namespace openglyph {

/**
 * @brief Maintains a list of players
 */
class PlayerList final
{
public:
    class Builder final
    {
    public:
        explicit Builder(const FactionList& factions);

        void add_player(Player&& player);

        /**
         * Marks the specified player as the "local" player.
         *
         * \throw ArgumentError if the index is out of bounds.
         */
        void set_local_player(std::size_t index);

        /**
         * Builds and returns the player list
         *
         * \throws InvalidOperationException if the local player was not set before this call.
         */
        PlayerList build() &&;

    private:
        const FactionList&         m_factions;
        std::vector<Player>        m_players;
        std::optional<std::size_t> m_local_player_index;
    };

    [[nodiscard]] std::size_t player_count() const noexcept
    {
        return m_players.size();
    }

    Player& player(std::size_t index) noexcept
    {
        assert(index < m_players.size());
        return m_players[index];
    }

    const Player& player(std::size_t index) const noexcept
    {
        assert(index < m_players.size());
        return m_players[index];
    }

    /**
     * Returns a range view for all players
     */
    [[nodiscard]] auto all() const noexcept
    {
        return m_players | std::views::all;
    }

    /**
     * Returns the local player.
     *
     * The local player is the player who's controlling this instance of the game.
     * By default, the player at index 0 is the local player.
     */
    [[nodiscard]] const Player& local_player() const noexcept
    {
        assert(m_local_player_index < m_players.size());
        return m_players[m_local_player_index];
    }

    /**
     * Returns the local player.
     *
     * The local player is the player who's controlling this instance of the game.
     * By default, the player at index 0 is the local player.
     */
    [[nodiscard]] Player& local_player() noexcept
    {
        assert(m_local_player_index < m_players.size());
        return m_players[m_local_player_index];
    }

private:
    PlayerList(std::vector<Player> players, std::size_t local_player_index);

    std::vector<Player> m_players;
    std::size_t         m_local_player_index;
};
} // namespace openglyph
