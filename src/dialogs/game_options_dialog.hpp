#pragma once

#include "game_modes/game_mode.hpp"

namespace openeaw {

class GameOptionsDialog final : public openglyph::ui::DialogController
{
public:
    // Name of the dialog definition to load
    static constexpr std::string_view RESOURCE_NAME = "IDD_GAME_OPTIONS_DIALOG";

    // Where to place the dialog on the screen
    static constexpr openglyph::ui::DialogPosition DIALOG_POSITION =
        openglyph::ui::DialogPosition::center;

    using QuitGameHandler = std::function<void()>;

    explicit GameOptionsDialog(openglyph::ui::Dialog& dialog_, QuitGameHandler quit_game);
};

} // namespace openeaw
