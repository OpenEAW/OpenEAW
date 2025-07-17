cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

project(OpenGlyph CXX)

# External libraries
find_package(rapidxml REQUIRED)

add_library(${PROJECT_NAME}
    src/assets/asset_cache.cpp
    src/assets/asset_loader.cpp
    src/assets/io/map.cpp
    src/game/game_object_type_store.cpp
    src/game/scene_renderer.cpp
    src/game/scene.cpp
    src/game/tactical_camera_store.cpp
    src/io/chunk_reader.cpp
    src/renderer/io/material.cpp
    src/renderer/io/model.cpp
    src/renderer/material_store.cpp
    src/renderer/model_creator.cpp
    src/parser/parsers.cpp
    src/parser/xml_parser.cpp
    src/ui/input.cpp
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    khepri::khepri
    rapidxml::rapidxml
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    include
)

add_library(openglyph::openglyph ALIAS OpenGlyph)
