#include "game_modes/tactical_game_mode.hpp"
#include "skirmish_staging_dialog.hpp"

#include <khepri/ui/listbox.hpp>

namespace openeaw {

SkirmishStagingDialog::SkirmishStagingDialog(openglyph::ui::Dialog& dialog_,
                                             GameModeStack&         game_mode_stack)
    : DialogController(dialog_)
{
    // Hide unused elements
    dialog().visible("IDC_SCENARIO_DESCRIPTION_TEXT_BOX", false);

    // We don't support this yet
    dialog().disable("IDC_SELECT_OPTIONS");
    dialog().disable("IDC_SELECT_OFFICIAL_MAPS");
    dialog().disable("IDC_SELECT_CUSTOM_MAPS");

    dialog().on_button("IDC_SKIRMISH_START_GAME_BUTTON", [&] {
        game_mode_stack.activate_game_mode<openeaw::TacticalGameMode>();
        dialog().close();
    });

    dialog().on_button("IDC_CANCEL_BUTTON", [&] { dialog().close(); });

    dialog().items("IDC_STAGING_MAP_LIST", std::vector<std::u16string>{u"Alderaan"});

    const unsigned int num_players = 2;
    const unsigned int max_players = 8;
    for (unsigned int i = 0; i < max_players; ++i) {
        const auto player_widget_name  = fmt::format(FMT_STRING("IDC_PLAYER_NAME_{}"), i);
        const auto faction_widget_name = fmt::format(FMT_STRING("IDC_PLAYER_FACTION_{}"), i);
        const auto color_widget_name   = fmt::format(FMT_STRING("IDC_PLAYER_COLOR_{}"), i);
        const bool is_used             = (i < num_players);

        dialog().enabled(player_widget_name, is_used);
        dialog().enabled(faction_widget_name, is_used);
        dialog().enabled(color_widget_name, is_used);
        if (is_used) {
            dialog().items(player_widget_name,
                           std::vector<std::u16string>{u"Player A", u"Player B", u"Player C"});
            dialog().items(faction_widget_name,
                           std::vector<std::u16string>{u"Faction A", u"Faction B", u"Faction C"});
            // Use U+25A0: BLACK SQUARE for the color indicator.
            dialog().items(color_widget_name,
                           std::vector<std::u16string>{u"\u25A0", u"\u25A0", u"\u25A0"});
        } else {
            dialog().clear(player_widget_name);
            dialog().clear(faction_widget_name);
            dialog().clear(color_widget_name);
        }
    }
}

} // namespace openeaw
