#pragma once

#include "game_data.hpp"

#include <khepri/application/window.hpp>
#include <khepri/renderer/renderer.hpp>
#include <openglyph/assets/asset_cache.hpp>
#include <openglyph/assets/asset_loader.hpp>

namespace openeaw {

class GameController;

/**
 * Structure containing the context for the game: all important objects or services that any part of
 * the game might need.
 */
struct GameContext
{
    /// The window running the game
    khepri::application::Window& window;

    /// The object to load assets with
    openglyph::AssetLoader& asset_loader;

    /// The asset cache
    openglyph::AssetCache& asset_cache;

    /// The renderer for the game
    khepri::renderer::Renderer& renderer;

    // The game controller
    GameController& game_controller;

    /// Static game data read from configuration files
    const GameData& game_data;
};

} // namespace openeaw
