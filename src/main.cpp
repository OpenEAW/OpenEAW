#include "version.hpp"

#include <fmt/format.h>
#include <khepri/application/console_logger.hpp>
#include <khepri/application/current_directory.hpp>
#include <khepri/application/exceptions.hpp>
#include <khepri/application/window.hpp>
#include <khepri/log/log.hpp>
#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/diligent/renderer.hpp>
#include <khepri/renderer/io/shader.hpp>
#include <khepri/renderer/io/texture.hpp>
#include <khepri/scene/scene_object.hpp>
#include <khepri/utility/cache.hpp>
#include <khepri/utility/string.hpp>
#include <openglyph/io/mega_filesystem.hpp>
#include <openglyph/assets/asset_cache.hpp>
#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/assets/io/map.hpp>
#include <openglyph/game/behaviors/render_behavior.hpp>
#include <openglyph/game/game_object_type_store.hpp>
#include <openglyph/game/scene.hpp>
#include <openglyph/game/scene_renderer.hpp>
#include <openglyph/renderer/io/material.hpp>
#include <openglyph/renderer/io/model.hpp>
#include <openglyph/renderer/material_store.hpp>
#include <openglyph/renderer/model_creator.hpp>

#include <cstdlib>
#include <cxxopts.hpp>

namespace {
constexpr auto APPLICATION_NAME = "OpenEAW";
constexpr auto PROGRAM_NAME     = "OpenEAW";

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
                  << "Use '--help' for help." << std::endl;
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
        khepri::to_radians(90.0f),
        0,
        static_cast<float>(render_size.width) / render_size.height,
        10.0f,
        100000.0f};
    return khepri::renderer::Camera{properties};
}

} // namespace

int main(int argc, const char* argv[])
{
#ifndef NDEBUG
    // In debug mode we create a console to log
    khepri::application::ConsoleLogger console_logger;
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
        data_paths.push_back(curdir);

        LOG.info("Starting up in \"{}\" with {} data path(s):", curdir.string(), data_paths.size());
        for (const auto& data_path : data_paths) {
            LOG.info(" - {}", data_path);
        }

        openglyph::AssetLoader asset_loader(data_paths);

        khepri::application::Window          window(APPLICATION_NAME);
        khepri::renderer::diligent::Renderer renderer(window.native_handle());
        window.add_size_listener([&] { renderer.render_size(window.render_size()); });
        renderer.render_size(window.render_size());

        openglyph::AssetCache          asset_cache(asset_loader, renderer);
        openglyph::GameObjectTypeStore game_object_types(asset_loader, "GameObjectFiles.xml");
        openglyph::Environment         environment{};
        
        if (auto stream = asset_loader.open_map("_SPACE_PLANET_ALDERAAN_01")) {
            const auto map = openglyph::io::read_map(*stream);
            if (!map.environments.empty()) {
                assert(map.active_environment < map.environments.size());
                environment = map.environments[map.active_environment];
            }
        }

        openglyph::Scene         scene(asset_cache, game_object_types, environment);
        openglyph::SceneRenderer scene_renderer(renderer);

        while (!window.should_close()) {
            khepri::application::Window::poll_events();
            renderer.clear(khepri::renderer::Renderer::clear_all);

            khepri::renderer::Camera camera = create_camera(renderer.render_size());
            scene_renderer.render_scene(scene, camera);

            // Presenting the rendered content has two different approaches, depending on the
            // rendering system: For OpenGL, the window needs to swap the front and back buffers.
            // For other systems, the renderer handles the presentation.
            if (window.use_swap_buffers()) {
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