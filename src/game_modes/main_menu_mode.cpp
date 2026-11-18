#include "dialogs/main_menu_dialog.hpp"
#include "main_menu_mode.hpp"

namespace openeaw {

MainMenuMode::MainMenuMode(const GameContext& context) : GameMode(context)
{
    ui().activate_dialog<openeaw::MainMenuDialog>(context);
}

void MainMenuMode::update(double delta_time)
{
    GameMode::update(delta_time);

    // If there's no more dialogs left, the main menu is done and we can close this game mode.
    if (!ui().has_active_dialogs()) {
        close();
    }
}

} // namespace openeaw
