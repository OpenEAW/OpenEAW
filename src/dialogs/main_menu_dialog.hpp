#pragma once

#include "single_player_menu_dialog.hpp"

#include <openglyph/ui/context.hpp>
#include <openglyph/ui/dialogs.hpp>

namespace openeaw {

// Logic for the main menu dialog
class MainMenuDialog final : public openglyph::ui::DialogController
{
public:
    // Name of the dialog definition to load
    static constexpr std::string_view RESOURCE_NAME = "IDD_MAIN_MENU_DIALOG";

    // Where to place the dialog on the screen
    static constexpr openglyph::ui::DialogPosition DIALOG_POSITION =
        openglyph::ui::DialogPosition::top_right;

    // Called before the dialog is created and added to the canvas
    static void before_dialog_creation(openglyph::ui::Context& context);

    explicit MainMenuDialog(openglyph::ui::Dialog& dialog_, GameModeStack& game_mode_stack);

    void on_deactivated() override;
    void on_activated() override;
};

} // namespace openeaw
