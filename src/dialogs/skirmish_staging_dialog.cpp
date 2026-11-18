#include "game_controller.hpp"
#include "game_modes/tactical_game_mode.hpp"
#include "skirmish_staging_dialog.hpp"

#include <khepri/ui/listbox.hpp>
#include <khepri/utility/ranges.hpp>

#include <format>
#include <ranges>

namespace openeaw {
namespace {
// Functor for determining if a faction is playable
[[nodiscard]] constexpr bool IsPlayable(const openglyph::Faction& faction) noexcept
{
    return faction.is_playable;
}

// Functor for getting the localized string with given ID
class GetString final
{
public:
    explicit GetString(const GameContext& context) : m_context(context) {}

    [[nodiscard]] std::u16string_view operator()(const auto& name_id) const
    {
        return m_context.game_data.string_database.get(name_id).value_or(u"");
    }

private:
    const GameContext& m_context;
};
} // namespace

SkirmishStagingDialog::SkirmishStagingDialog(openglyph::ui::Dialog& dialog_,
                                             const GameContext&     context)
    : DialogController(dialog_)
    , m_context(context)
    , m_playable_factions(m_context.game_data.factions.all() | std::views::filter(IsPlayable) |
                          khepri::views::to_pointers | khepri::ranges::to<std::vector>())
{
    using namespace std::literals;

    // Hide unused elements
    dialog().visible("IDC_SCENARIO_DESCRIPTION_TEXT_BOX", false);

    // We don't support this yet
    dialog().disable("IDC_SELECT_OPTIONS");
    dialog().disable("IDC_SELECT_OFFICIAL_MAPS");
    dialog().disable("IDC_SELECT_CUSTOM_MAPS");

    dialog().on_button("IDC_SKIRMISH_START_GAME_BUTTON", [&] {
        TacticalGameMode::GameOptions options;

        m_players.reset();
        openglyph::PlayerList::Builder player_list_builder(m_context.game_data.factions);
        if (const auto player_faction_index = dialog().selection("IDC_PLAYER_FACTION_0")) {
            const auto* faction =
                m_context.game_data.factions.get(m_playable_factions[*player_faction_index]->id);
            assert(faction != nullptr);
            player_list_builder.add_player(openglyph::Player(*faction));
        }
        // The first player is always the local player
        player_list_builder.set_local_player(0);
        m_players = std::move(player_list_builder).build();
        context.game_controller.activate_game_mode<TacticalGameMode>(*m_players, options);
        dialog().close();
    });

    dialog().on_button("IDC_CANCEL_BUTTON", [&] { dialog().close(); });

    dialog().items("IDC_STAGING_MAP_LIST", std::vector<std::u16string_view>{u"Alderaan"});

    const unsigned int num_players = 2;
    const unsigned int max_players = 8;
    for (unsigned int i = 0; i < max_players; ++i) {
        const auto player_widget_name  = std::format("IDC_PLAYER_NAME_{}", i);
        const auto faction_widget_name = std::format("IDC_PLAYER_FACTION_{}", i);
        const auto color_widget_name   = std::format("IDC_PLAYER_COLOR_{}", i);
        const bool is_used             = (i < num_players);

        dialog().enabled(player_widget_name, is_used);
        dialog().enabled(faction_widget_name, is_used);
        dialog().enabled(color_widget_name, is_used);
        if (is_used) {
            dialog().items(player_widget_name,
                           std::vector{u"Player A"sv, u"Player B"sv, u"Player C"sv});
            dialog().select(player_widget_name, 0);
            dialog().items(
                faction_widget_name,
                m_playable_factions | khepri::views::pluck<&openglyph::Faction::display_name_id> |
                    std::views::transform(GetString(context)) | khepri::ranges::to<std::vector>());
            dialog().select(faction_widget_name, 0);
            // Use U+25A0: BLACK SQUARE for the color indicator.
            dialog().items(color_widget_name, std::vector{u"\u25A0"sv, u"\u25A0"sv, u"\u25A0"sv});
            dialog().select(color_widget_name, 0);
        } else {
            dialog().clear(player_widget_name);
            dialog().clear(faction_widget_name);
            dialog().clear(color_widget_name);
        }
    }
}

} // namespace openeaw
