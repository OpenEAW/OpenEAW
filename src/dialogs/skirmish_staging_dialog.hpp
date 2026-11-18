#pragma once

#include "game_context.hpp"

#include <openglyph/game/player_list.hpp>
#include <openglyph/ui/context.hpp>
#include <openglyph/ui/dialogs.hpp>

#include <optional>

namespace openeaw {

// Logic for the single player skirmish staging dialog
class SkirmishStagingDialog final : public openglyph::ui::DialogController
{
public:
    // Name of the dialog definition to load
    static constexpr std::string_view RESOURCE_NAME = "IDD_SKIRMISH_STAGING_DIALOG";

    // Where to place the dialog on the screen
    static constexpr openglyph::ui::DialogPosition DIALOG_POSITION =
        openglyph::ui::DialogPosition::center;

    explicit SkirmishStagingDialog(openglyph::ui::Dialog& dialog_, const GameContext& context);

private:
    const GameContext&                   m_context;
    std::optional<openglyph::PlayerList> m_players;

    std::vector<const openglyph::Faction*> m_playable_factions;
};

} // namespace openeaw
