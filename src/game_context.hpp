#pragma once

#include <khepri/application/window.hpp>
#include <khepri/renderer/renderer.hpp>
#include <openglyph/assets/asset_cache.hpp>
#include <openglyph/assets/asset_loader.hpp>

namespace openeaw {

/**
 * Structure containing the context for the game: all important objects or services that any part of
 * the game might need.
 */
struct GameContext
{
    khepri::application::Window& window;
    openglyph::AssetLoader&      asset_loader;
    openglyph::AssetCache&       asset_cache;
    khepri::renderer::Renderer&  renderer;
};

} // namespace openeaw
