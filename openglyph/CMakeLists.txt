cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

project(OpenGlyph CXX)

# External libraries
find_package(rapidxml REQUIRED)

add_library(${PROJECT_NAME}
    src/assets/asset_cache.cpp
    src/assets/asset_loader.cpp
    src/assets/mega_texture.cpp
    src/assets/string_database.cpp
    src/assets/io/map.cpp
    src/assets/io/mega_texture.cpp
    src/assets/io/string_database.cpp
    src/game/game_object_type_store.cpp
    src/game/scene_renderer.cpp
    src/game/scene.cpp
    src/game/tactical_camera_store.cpp
    src/io/chunk_reader.cpp
    src/io/mega_filesystem.cpp
    src/io/mega_file.cpp
    src/renderer/io/graphics_pipeline_options.cpp
    src/renderer/io/material.cpp
    src/renderer/io/model.cpp
    src/renderer/io/render_pipeline.cpp
    src/renderer/material_store.cpp
    src/renderer/model_creator.cpp
    src/renderer/render_pipeline_store.cpp
    src/parser/parsers.cpp
    src/parser/xml_parser.cpp
    src/ui/context.cpp
    src/ui/dialog_creator.cpp
    src/ui/dialog_store.cpp
    src/ui/dialogs.cpp
    src/ui/input.cpp
    src/ui/style_store.cpp
    src/ui/ui.cpp
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
