#pragma once

#include <khepri/utility/string.hpp>

#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/game/faction.hpp>

#include <map>
#include <ranges>
#include <string_view>

namespace openglyph {

/**
 * @brief Loads and stores factions
 *
 * This class holds a list of factions, their properties and their relations.
 */
class FactionList final
{
public:
    /**
     * @brief Constructs a FactionList by loading from XML configuration.
     *
     * The AssetLoader is used to load the @a index_filename configuration file, which is an XML
     * file containing references to other XML files with the actual Faction definitions.
     *
     * All Factions are created from these definitions and stored in this object.
     */
    FactionList(AssetLoader& asset_loader, std::string_view index_filename);

    FactionList(const FactionList&)            = delete;
    FactionList(FactionList&&)                 = delete;
    FactionList& operator=(const FactionList&) = delete;
    FactionList& operator=(FactionList&&)      = delete;

    /**
     * @brief Finds a Faction with a given ID.
     *
     * The lifetime of the returned reference is tied to the lifetime of the store.
     *
     * @note the name lookup is case insensitive.
     */
    [[nodiscard]] const Faction* get(std::string_view id) const noexcept;

    /**
     * @brief Gets the number of factions.
     */
    [[nodiscard]] std::size_t faction_count() const noexcept
    {
        return m_factions.size();
    }

    /**
     * @brief Finds a Faction by a given index
     *
     * The lifetime of the returned reference is tied to the lifetime of the store.
     *
     * The index should be between 0 (inclusive) and #faction_count (exclusive).
     * If the index is out of bounds, this returns nullptr.
     */
    [[nodiscard]] const Faction* get(std::size_t index) const noexcept;

    /**
     * @brief Returns all factions
     *
     * Returns all factions as a range view.
     */
    [[nodiscard]] auto all() const noexcept
    {
        return m_factions | std::views::all;
    }

private:
    void read_factions(khepri::io::Stream& stream);

    std::vector<Faction>                                                    m_factions;
    std::map<std::string_view, const Faction*, khepri::CaseInsensitiveLess> m_name_index;
};

} // namespace openglyph