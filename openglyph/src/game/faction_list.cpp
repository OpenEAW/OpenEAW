#include <openglyph/game/faction_list.hpp>
#include <openglyph/parser/parsers.hpp>
#include <openglyph/parser/xml_parser.hpp>

namespace openglyph {
namespace {
Faction read_faction(const XmlParser::Node& node)
{
    Faction faction;
    faction.id              = parse<std::string_view>(require_attribute(node, "Name"));
    faction.display_name_id = parse<std::string_view>(optional_child(node, "Text_ID")).value_or("");
    faction.is_playable     = parse<bool>(optional_child(node, "Is_Playable")).value_or(false);
    return faction;
}
} // namespace

void FactionList::read_factions(khepri::io::Stream& stream)
{
    const XmlParser parser(stream);
    if (const auto& root = parser.root()) {
        for (const auto& node : root->nodes()) {
            auto faction = read_faction(node);
            m_factions.push_back(std::move(faction));
        }
    }
}

FactionList::FactionList(AssetLoader& asset_loader, std::string_view index_filename)
{
    if (auto index_stream = asset_loader.open_config(index_filename)) {
        const XmlParser parser(*index_stream);
        if (const auto& root = parser.root()) {
            for (const auto& file : root->nodes()) {
                if (auto config_stream = asset_loader.open_config(file.value())) {
                    read_factions(*config_stream);
                }
            }
        }
    }

    // After reading all factions, build the name index
    for (const auto& faction : m_factions) {
        m_name_index.emplace(faction.id, &faction);
    }
}

[[nodiscard]] const Faction* FactionList::get(std::string_view id) const noexcept
{
    if (const auto it = m_name_index.find(id); it != m_name_index.end()) {
        return it->second;
    }
    return nullptr;
}

[[nodiscard]] const Faction* FactionList::get(std::size_t index) const noexcept
{
    return (index < m_factions.size()) ? &m_factions[index] : nullptr;
}

} // namespace openglyph