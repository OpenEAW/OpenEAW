#include "main_menu_dialog.hpp"

#include <khepri/ui/image.hpp>
#include <openglyph/ui/ui.hpp>

namespace openeaw {

void MainMenuDialog::before_dialog_creation(openglyph::ui::Context& context)
{
    // Add the splash image for the main menu
    const auto splash_texture = context.get_texture("Menuback_Overlay");

    auto image = std::make_shared<khepri::ui::Image>(khepri::ui::Layout::fill());
    image->style({context.sprite_material_id(), {}});
    image->image(splash_texture.texture_id, splash_texture.tex_rect);
    context.canvas().add(std::move(image));
}

MainMenuDialog::MainMenuDialog(openglyph::ui::Dialog& dialog_, GameModeStack& game_mode_stack)
    : DialogController(dialog_)
{
    // For now we don't support anything yet, other than exiting the game.
    dialog_.disable("IDC_MAIN_MENU_TUTORIAL");
    dialog_.disable("IDC_MAIN_MENU_MULTIPLAYER");
    dialog_.disable("IDC_LOAD_GAME_MAIN");
    dialog_.disable("IDC_OPTIONS");
    dialog_.disable("IDC_MENU_CREDITS");

    // FoC has a "play demo" button for demo builds. We don't want to see that.
    dialog_.disable("IDC_MAIN_MENU_PLAY_DEMO");
    dialog_.visible("IDC_MAIN_MENU_PLAY_DEMO", false);

    dialog_.on_button("IDC_MAIN_MENU_SINGLE_PLAYER_GAMES", [&] {
        dialog().ui().activate_dialog<SinglePlayerMenuDialog>(game_mode_stack);
    });

    dialog_.on_button("IDC_MAIN_MENU_EXIT2", [&] { dialog().close(); });
}

void MainMenuDialog::on_deactivated()
{
    // When going into a submenu, the submenu replaces this menu. Since there's transparency
    // involved and we don't want this menu to show up under the new menu, we hide it until
    // we're activated again.
    dialog().hide();
}

void MainMenuDialog::on_activated()
{
    dialog().show();
}

} // namespace openeaw
