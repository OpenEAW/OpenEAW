#pragma once

#include "game_context.hpp"
#include "game_mode.hpp"

namespace openeaw {

/**
 * This class is the "game mode" for the main menu.
 */
class MainMenuMode final : public GameMode
{
public:
    explicit MainMenuMode(const GameContext& context);

    void update(double delta_time) override;
};

} // namespace openeaw
