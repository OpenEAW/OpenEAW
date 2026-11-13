#pragma once

#include <openglyph/assets/string_database.hpp>
#include <openglyph/game/game_object_type_store.hpp>
#include <openglyph/game/tactical_camera_store.hpp>

namespace openeaw {

// Collection of all immutable game data that is loaded from configuration or other data files.
struct GameData
{
    openglyph::StringDatabase      string_database;
    openglyph::GameObjectTypeStore game_object_types;
    openglyph::TacticalCameraStore tactical_camera_store;
};

} // namespace openeaw