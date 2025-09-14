#include "version.hpp"
#include "./io/path_manager.hpp"

#include <fmt/format.h>
#include <khepri/adapters/window_input.hpp>
#include <khepri/application/console_logger.hpp>
#include <khepri/application/current_directory.hpp>
#include <khepri/application/exceptions.hpp>
#include <khepri/application/window.hpp>
#include <khepri/game/rts_camera.hpp>
#include <khepri/log/log.hpp>
#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/diligent/renderer.hpp>
#include <khepri/renderer/io/shader.hpp>
#include <khepri/renderer/io/texture.hpp>
#include <khepri/scene/scene_object.hpp>
#include <khepri/utility/cache.hpp>
#include <khepri/utility/string.hpp>
#include <openglyph/assets/asset_cache.hpp>
#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/assets/io/map.hpp>
#include <openglyph/game/behaviors/marker_behavior.hpp>
#include <openglyph/game/behaviors/render_behavior.hpp>
#include <openglyph/game/game_object_type_store.hpp>
#include <openglyph/game/scene.hpp>
#include <openglyph/game/scene_renderer.hpp>
#include <openglyph/game/tactical_camera_store.hpp>
#include <openglyph/io/mega_filesystem.hpp>
#include <openglyph/renderer/io/material.hpp>
#include <openglyph/renderer/io/model.hpp>
#include <openglyph/renderer/material_store.hpp>
#include <openglyph/renderer/model_creator.hpp>
#include <openglyph/steam/steam_paths.hpp>
#include <openglyph/ui/input.hpp>

#include <cstdlib>
#include <cxxopts.hpp>

namespace {
constexpr auto APPLICATION_NAME = "OpenEAW";
constexpr auto PROGRAM_NAME     = "OpenEAW";

// Time, in seconds, between each 'game logic' update step.
constexpr auto UPDATE_STEP_TIME = 1.0 / 60;

constexpr khepri::log::Logger LOG("openeaw");

auto full_version_string()
{
    return fmt::format(FMT_STRING("{} {}"), APPLICATION_NAME, to_string(openeaw::version()));
}

struct CmdlineArgs
{
    bool show_help{false};

    bool show_version{false};

    std::vector<std::filesystem::path> modpaths;
};

auto create_cmdline_options()
{
    cxxopts::Options options(PROGRAM_NAME,
                             fmt::format("{} {}", APPLICATION_NAME, to_string(openeaw::version())));

    auto adder = options.add_options();
    adder("h,help", "display this help and exit");
    adder("v,version", "display version information");
    adder("m,modpaths", "comma-separate list of paths to preferred source of game data",
          cxxopts::value<std::string>());
    return options;
}

std::optional<CmdlineArgs> parse_cmdline_arguments(int argc, const char* argv[])
{
    try {
        auto options = create_cmdline_options();
        options.parse_positional({"input", "output"});
        auto result = options.parse(argc, argv);

        CmdlineArgs args;
        if (result.count("help") != 0) {
            args.show_help = true;
        }

        if (result.count("version") != 0) {
            args.show_version = true;
        }

        if (result.count("modpaths") != 0) {
            const auto str = result["modpaths"].as<std::string>();
            for (const auto& path : khepri::split(str, ",")) {
                args.modpaths.emplace_back(path);
            }
        }
        return args;
    } catch (const cxxopts::OptionException& e) {
        std::cerr << "error: " << e.what() << "\n"
                  << "Use '--help' for help.\n";
    }
    return {};
}

auto create_camera(const khepri::Size& render_size)
{
    const struct khepri::renderer::Camera::Properties properties = {
        khepri::renderer::Camera::Type::perspective,
        {100, 100, 150},
        {0, 0, 0},
        {0, 0, 1},
        khepri::to_radians(90.0F),
        0,
        static_cast<float>(render_size.width) / static_cast<float>(render_size.height),
        10.0F,
        100000.0F};
    return khepri::renderer::Camera{properties};
}

std::unique_ptr<openglyph::Scene>
CreateScene(std::string_view map_name, openglyph::AssetLoader& asset_loader,
            openglyph::AssetCache&                asset_cache,
            const openglyph::GameObjectTypeStore& game_object_types,
            khepri::game::RtsCameraController&    camera)
{
    if (auto stream = asset_loader.open_map(map_name)) {
        const auto             map = openglyph::io::read_map(*stream);
        openglyph::Environment environment{};
        if (!map.environments.empty()) {
            assert(map.active_environment < map.environments.size());
            environment = map.environments[map.active_environment];
        }

        auto scene =
            std::make_unique<openglyph::Scene>(asset_cache, game_object_types, environment);

        for (const auto& obj : map.objects) {
            if (const auto* type = game_object_types.get(obj.type_crc)) {
                auto object = std::make_shared<khepri::scene::SceneObject>();

                // Store a (dumb, non-owning) reference to the GameObjectType
                object->user_data(type);

                if (const auto* render_model =
                        asset_cache.get_render_model(type->space_model_name)) {
                    auto& behavior =
                        object->create_behavior<openglyph::RenderBehavior>(*render_model);
                    behavior.scale(type->scale_factor);
                    if (type->is_in_background) {
                        behavior.render_layer(openglyph::RenderBehavior::RenderLayer::background);
                    }
                }

                if (type->is_marker) {
                    object->create_behavior<openglyph::MarkerBehavior>();
                }

                object->rotation(obj.facing);
                object->position(obj.position);
                scene->add_object(std::move(object));
            }
        }

        // Find the first "player 0" marker to place the camera at.
        for (const auto& object : scene->objects<openglyph::MarkerBehavior>()) {
            if (const auto** type_ptr = object->user_data<const openglyph::GameObjectType*>()) {
                if (khepri::case_insensitive_equals((*type_ptr)->name,
                                                    "Player_0_Spawn_Point_Marker")) {
                    camera.target({object->position().x, object->position().y});
                    break;
                }
            }
        }

        return scene;
    }
    return {};
}

} // namespace

int main(int argc, const char* argv[])
{
#ifndef NDEBUG
    // In debug mode we create a console to log
    const khepri::application::ConsoleLogger console_logger;
#endif

    khepri::application::ExceptionHandler exception_handler("main");

    auto result = exception_handler.invoke([&]() {
        const auto args = parse_cmdline_arguments(argc, argv);
        if (!args) {
            return EXIT_FAILURE;
        }

        if (args->show_help) {
            std::cout << create_cmdline_options().help() << "\n";
            return EXIT_SUCCESS;
        }

        if (args->show_version) {
            std::cout << full_version_string() << '\n';
            return EXIT_SUCCESS;
        }

        LOG.info("Running {}", full_version_string());

        const auto curdir     = khepri::application::get_current_directory();
        auto       data_paths = args->modpaths;
        data_paths.push_back(openeaw::io::PathManager::get_install_path(openeaw::io::steam) / "GameData");
        data_paths.push_back(curdir);

        LOG.info("Starting up in \"{}\" with {} data path(s):", curdir.string(), data_paths.size());
        for (const auto& data_path : data_paths) {
            LOG.info(" - {}", data_path);
        }

        openglyph::AssetLoader asset_loader(std::move(data_paths));

        khepri::application::Window window(APPLICATION_NAME);

        // Note: Empire at War was written for DX9 and does not natively support sRGB mode. Textures
        // are read & modified in linear space and (roughly) gamma-corrected in the shader. Thus,
        // the output format should be in linear space.
        khepri::renderer::diligent::Renderer renderer(window.native_handle(),
                                                      khepri::renderer::ColorSpace::linear);

        khepri::renderer::Camera camera = create_camera(window.render_size());
        window.add_size_listener([&] {
            const auto render_size = window.render_size();
            renderer.render_size(render_size);
            camera.aspect(static_cast<float>(render_size.width) / render_size.height);
        });
        renderer.render_size(window.render_size());

        openglyph::AssetCache                asset_cache(asset_loader, renderer);
        const openglyph::GameObjectTypeStore game_object_types(asset_loader, "GameObjectFiles.xml");
        const openglyph::TacticalCameraStore tactical_camera_store(asset_loader,
                                                                   "TacticalCameras.xml");
        khepri::game::RtsCameraController    rts_camera = [&] {
            if (auto rts_camera = tactical_camera_store.create("Space_Mode", camera)) {
                return std::move(*rts_camera);
            }
            return khepri::game::RtsCameraController(camera, {0, 0});
        }();

        khepri::WindowInputEventGenerator       input_event_generator(window);
        openglyph::ui::TacticalModeInputHandler tactical_mode_input_handler(rts_camera, window);
        input_event_generator.AddEventHandler(&tactical_mode_input_handler);

        std::unique_ptr<openglyph::Scene> scene = CreateScene(
            "_MP_SPACE_ALDERAAN", asset_loader, asset_cache, game_object_types, rts_camera);

        auto render_pipeline = asset_cache.get_render_pipeline("Default");
        if (!render_pipeline) {
            // We can't render without a pipeline, so this is a fatal error
            throw std::runtime_error("Unable to load default render pipeline");
        }
        openglyph::SceneRenderer scene_renderer(renderer, *render_pipeline);

        std::chrono::steady_clock::time_point last_update_time = std::chrono::steady_clock::now();
        double                                unhandled_update_time = 0.0;

        while (!window.should_close()) {
            khepri::application::Window::poll_events();

            const auto current_time      = std::chrono::steady_clock::now();
            const auto delta_update_time = std::chrono::duration_cast<std::chrono::microseconds>(
                                               current_time - last_update_time)
                                               .count() /
                                           1000000.0;
            if (delta_update_time >= UPDATE_STEP_TIME) {
                unhandled_update_time += delta_update_time;
                while (unhandled_update_time >= UPDATE_STEP_TIME) {
                    rts_camera.update(UPDATE_STEP_TIME);
                    unhandled_update_time -= UPDATE_STEP_TIME;
                }
                last_update_time = current_time;
            }

            renderer.clear(khepri::renderer::Renderer::clear_all);
            if (scene) {
                scene_renderer.render_scene(*scene, camera);
            }

            // Presenting the rendered content has two different approaches, depending on the
            // rendering system: For OpenGL, the window needs to swap the front and back
            // buffers. For other systems, the renderer handles the presentation.
            if (khepri::application::Window::use_swap_buffers()) {
                window.swap_buffers();
            } else {
                renderer.present();
            }
        }

        LOG.info("Shutting down");
        return EXIT_SUCCESS;
    });
    return result ? *result : EXIT_FAILURE;
}