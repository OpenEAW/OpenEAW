#include "game_options_dialog.hpp"

namespace openeaw {

GameOptionsDialog::GameOptionsDialog(openglyph::ui::Dialog& dialog_, QuitGameHandler quit_game)
    : DialogController(dialog_)
{
    assert(quit_game);

    dialog().disable("IDC_LOAD_GAME");
    dialog().disable("IDC_SAVE_GAME");
    dialog().disable("IDC_INGAME_OPTIONS");

    dialog().on_button("IDC_GAME_OPTIONS_QUIT", [=] { quit_game(); });

    dialog().on_button("IDC_GAME_OPTIONS_RESUME", [this] { dialog().close(); });
}

} // namespace openeaw
