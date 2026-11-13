#pragma once

#include "game_mode_stack.hpp"

#include <openglyph/ui/context.hpp>
#include <openglyph/ui/dialogs.hpp>

namespace openeaw {

// Logic for the single player menu dialog
class SinglePlayerMenuDialog final : public openglyph::ui::DialogController
{
public:
    // Name of the dialog definition to load
    static constexpr std::string_view RESOURCE_NAME = "IDD_SINGLE_PLAYER_GAMES_MENU_DIALOG";

    // Where to place the dialog on the screen
    static constexpr openglyph::ui::DialogPosition DIALOG_POSITION =
        openglyph::ui::DialogPosition::top_right;

    explicit SinglePlayerMenuDialog(openglyph::ui::Dialog& dialog_, GameModeStack& game_mode_stack);
};

} // namespace openeaw
