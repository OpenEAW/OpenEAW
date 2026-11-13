#include "single_player_menu_dialog.hpp"
#include "skirmish_staging_dialog.hpp"

#include <openglyph/ui/ui.hpp>

namespace openeaw {

SinglePlayerMenuDialog::SinglePlayerMenuDialog(openglyph::ui::Dialog& dialog_,
                                               GameModeStack&         game_mode_stack)
    : DialogController(dialog_)
{
    // Hide debug buttons
    dialog_.visible("IDC_DEBUG_LOAD_SKIRMISH_MAP_BUTTON", false);

    dialog_.on_button("IDC_SKIRMISH_GAMES_BUTTON", [&] {
        dialog().ui().activate_dialog<SkirmishStagingDialog>(game_mode_stack);
    });

    dialog_.on_button("IDC_BACK_BUTTON_MAIN", [&] { dialog().close(); });

    // For now we don't support these  yet, other than closing the menu.
    dialog_.disable("IDC_NEW_CAMPAIGN_BUTTON");
    dialog_.disable("IDC_CUSTOM_CAMPAIGN_BUTTON");
    dialog_.disable("IDC_MENU_CREDITS");
}

} // namespace openeaw
