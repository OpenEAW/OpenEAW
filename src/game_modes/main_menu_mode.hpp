#pragma once

#include "game_data.hpp"
#include "game_mode.hpp"
#include "game_mode_stack.hpp"

namespace openeaw {

/**
 * This class is the "game mode" for the main menu.
 */
class MainMenuMode final : public GameMode
{
public:
    MainMenuMode(const GameContext& context, const GameData& game_data,
                 GameModeStack& game_mode_stack);

    void update(double delta_time) override;
};

} // namespace openeaw
