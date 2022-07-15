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
#include <khepri/scene/scene.hpp>
#include <khepri/scene/scene_object.hpp>
#include <khepri/utility/cache.hpp>
#include <khepri/utility/string.hpp>
#include <khepri/version.hpp>
#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/game/behaviors/render_behavior.hpp>
#include <openglyph/game/scene_renderer.hpp>
#include <openglyph/renderer/io/material.hpp>
#include <openglyph/renderer/io/model.hpp>
#include <openglyph/renderer/material_store.hpp>
#include <openglyph/renderer/model_creator.hpp>
#include <openglyph/version.hpp>

#include <cstdlib>
#include <cxxopts.hpp>

namespace {
constexpr auto APPLICATION_NAME = "OpenEAW";
constexpr auto PROGRAM_NAME     = "OpenEAW";

constexpr khepri::log::Logger LOG("openeaw");

auto full_version_string()
{
    return fmt::format(FMT_STRING("{} {} (OpenGlyph {}, Khepri {})"), APPLICATION_NAME,
                       to_string(openeaw::version()), to_string(openglyph::version()),
                       to_string(khepri::version()));
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
        {100, -100, 150},
        {0, 0, 0},
        {0, 0, 1},
        khepri::to_radians(90.0f),
        0,
        static_cast<float>(render_size.width) / render_size.height,
        0.1f,
        40000.0f};
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

        openglyph::AssetLoader asset_loader(std::move(data_paths));

        khepri::application::Window          window(APPLICATION_NAME);
        khepri::renderer::diligent::Renderer renderer(
            Diligent::Win32NativeWindow(window.native_handle()));
        window.add_size_listener([&] { renderer.render_size(window.render_size()); });
        renderer.render_size(window.render_size());

        // Create the shader cache
        khepri::OwningCache<khepri::renderer::Shader> shader_cache(
            [&](std::string_view name) -> std::unique_ptr<khepri::renderer::Shader> {
                const auto& shader_desc_loader = [&](const std::filesystem::path& path)
                    -> std::optional<khepri::renderer::ShaderDesc> {
                    if (auto stream = asset_loader.open_shader(path.string())) {
                        return khepri::renderer::io::load_shader(*stream);
                    }
                    return {};
                };
                return renderer.create_shader(name, shader_desc_loader);
            });

        // Create the texture cache
        khepri::OwningCache<khepri::renderer::Texture> texture_cache(
            [&](std::string_view name) -> std::unique_ptr<khepri::renderer::Texture> {
                if (auto stream = asset_loader.open_texture(name)) {
                    auto texture_desc = khepri::renderer::io::load_texture(*stream);
                    return renderer.create_texture(texture_desc);
                }
                return {};
            });

        openglyph::renderer::MaterialStore materials(renderer, shader_cache.as_loader(),
                                                     texture_cache.as_loader());
        if (auto stream = asset_loader.open_config("Materials")) {
            materials.register_materials(openglyph::renderer::io::load_materials(*stream));
        } else {
            LOG.error("Unable to load rendering material definitions");
        }

        // Define the render model cache
        openglyph::renderer::ModelCreator model_creator(renderer, materials.as_loader(),
                                                        texture_cache.as_loader());

        khepri::OwningCache<openglyph::renderer::RenderModel> render_model_cache(
            [&](std::string_view name) -> std::unique_ptr<openglyph::renderer::RenderModel> {
                if (auto stream = asset_loader.open_model(name)) {
                    const auto model = openglyph::io::read_model(*stream);
                    return model_creator.create_model(model);
                }
                return {};
            });

        khepri::scene::Scene     scene;
        openglyph::SceneRenderer scene_renderer(renderer);

        const auto* render_model = render_model_cache.get("W_STARS_HIGH");
        assert(render_model != nullptr);

        auto stars = std::make_shared<khepri::scene::SceneObject>();
        stars->create_behavior<openglyph::RenderBehavior>(*render_model);
        scene.add_object(stars);

        while (!window.should_close()) {
            khepri::application::Window::poll_events();
            renderer.clear(khepri::renderer::Renderer::clear_all);

            khepri::renderer::Camera camera = create_camera(renderer.render_size());
            scene_renderer.render_scene(scene, camera);

            renderer.present();
        }

        LOG.info("Shutting down");
        return EXIT_SUCCESS;
    });
    return result ? *result : EXIT_FAILURE;
}