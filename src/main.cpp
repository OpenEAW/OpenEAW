#include "game_context.hpp"
#include "game_data.hpp"
#include "game_mode_stack.hpp"
#include "game_modes/main_menu_mode.hpp"
#include "version.hpp"

#include <fmt/format.h>
#include <khepri/adapters/window_input.hpp>
#include <khepri/application/console_logger.hpp>
#include <khepri/application/current_directory.hpp>
#include <khepri/application/exceptions.hpp>
#include <khepri/log/log.hpp>
#include <khepri/renderer/diligent/renderer.hpp>
#include <openglyph/assets/io/string_database.hpp>

#include <cstdlib>
#include <cxxopts.hpp>
#include <filesystem>
#include <vector>

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

enum class Language
{
    english,
    german,
    french,
    spanish,
    italian,
    japanese,
    korean,
    chinese,
    russian,
    polish,
    thai,
};

std::string get_string_database_path(Language language)
{
    std::vector<std::string> language_names{"ENGLISH", "GERMAN",   "FRENCH", "SPANISH",
                                            "ITALIAN", "JAPANESE", "KOREAN", "CHINESE",
                                            "RUSSIAN", "POLISH",   "THAI"};
    assert(static_cast<std::size_t>(language) < language_names.size());
    return fmt::format(FMT_STRING("MasterTextFile_{}.dat"),
                       language_names[static_cast<std::size_t>(language)]);
}

openglyph::StringDatabase read_string_database(openglyph::AssetLoader& asset_loader,
                                               Language                language)
{
    const auto filename = get_string_database_path(language);
    if (auto stream = asset_loader.open_text(filename)) {
        return openglyph::io::read_string_database(*stream);
    }

    // If we can't find the string database for the requested language, fall back to English. If
    // that fails too, return an empty string database.
    if (language != Language::english) {
        return read_string_database(asset_loader, Language::english);
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

        window.add_size_listener([&] { renderer.render_size(window.render_size()); });
        renderer.render_size(window.render_size());

        openglyph::AssetCache asset_cache(asset_loader, renderer);

        const openeaw::GameContext game_context{window, asset_loader, asset_cache, renderer};

        const openeaw::GameData game_data{
            read_string_database(asset_loader, Language::english),
            openglyph::GameObjectTypeStore(asset_loader, "GameObjectFiles.xml"),
            openglyph::TacticalCameraStore(asset_loader, "TacticalCameras.xml")};

        khepri::WindowInputEventGenerator input_event_generator(window);

        openeaw::GameModeStack game_mode_stack(game_context, game_data);
        input_event_generator.add_event_handler(&game_mode_stack);

        game_mode_stack.activate_game_mode<openeaw::MainMenuMode>(game_mode_stack);

        std::chrono::steady_clock::time_point last_update_time = std::chrono::steady_clock::now();
        double                                unhandled_update_time = 0.0;

        while (!window.should_close() && game_mode_stack.has_active_game_mode()) {
            khepri::application::Window::poll_events();

            const auto current_time      = std::chrono::steady_clock::now();
            const auto delta_update_time = std::chrono::duration_cast<std::chrono::microseconds>(
                                               current_time - last_update_time)
                                               .count() /
                                           1000000.0;
            if (delta_update_time >= UPDATE_STEP_TIME) {
                unhandled_update_time += delta_update_time;
                while (unhandled_update_time >= UPDATE_STEP_TIME) {
                    game_mode_stack.update(UPDATE_STEP_TIME);
                    unhandled_update_time -= UPDATE_STEP_TIME;
                }
                last_update_time = current_time;
            }

            renderer.clear(khepri::renderer::Renderer::clear_all);
            game_mode_stack.render();

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