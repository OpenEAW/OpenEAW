#include "dialogs/main_menu_dialog.hpp"
#include "main_menu_mode.hpp"

#include <khepri/ui/canvas.hpp>

namespace openeaw {

MainMenuMode::MainMenuMode(const GameContext& context, const GameData& game_data,
                           GameModeStack& game_mode_stack)
    : GameMode(context, game_data.string_database)
{
    ui().activate_dialog<openeaw::MainMenuDialog>(game_mode_stack);
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
