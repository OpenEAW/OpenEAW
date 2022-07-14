#include "version.hpp"

#include <fmt/format.h>
#include <khepri/application/console_logger.hpp>
#include <khepri/application/current_directory.hpp>
#include <khepri/application/exceptions.hpp>
#include <khepri/application/window.hpp>
#include <khepri/log/log.hpp>
#include <khepri/renderer/diligent/renderer.hpp>
#include <khepri/version.hpp>
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

std::optional<int> handle_cmdline_arguments(int argc, const char* argv[])
{
    try {
        cxxopts::Options options(
            PROGRAM_NAME, fmt::format("{} {}", APPLICATION_NAME, to_string(openeaw::version())));
        auto adder = options.add_options();
        adder("h,help", "display this help and exit");
        adder("v,version", "display version information");

        options.parse_positional({"input", "output"});
        auto result = options.parse(argc, argv);
        if (result.count("help") != 0) {
            std::cout << options.help() << "\n";
            return EXIT_SUCCESS;
        }

        if (result.count("version") != 0) {
            std::cout << full_version_string() << '\n';
            return EXIT_SUCCESS;
        }
    } catch (const cxxopts::OptionException& e) {
        std::cerr << "error: " << e.what() << "\n"
                  << "Use '--help' for help." << std::endl;
        return EXIT_FAILURE;
    }

    return {};
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
        if (auto result = handle_cmdline_arguments(argc, argv)) {
            return (int)*result;
        }
        LOG.info("Running {}", full_version_string());

        const auto curdir = khepri::application::get_current_directory();
        LOG.info("Starting up in \"{}\"", curdir.string());

        khepri::application::Window          window(APPLICATION_NAME);
        khepri::renderer::diligent::Renderer renderer(
            Diligent::Win32NativeWindow(window.native_handle()));
        window.add_size_listener([&] { renderer.render_size(window.render_size()); });
        renderer.render_size(window.render_size());

        while (!window.should_close()) {
            khepri::application::Window::poll_events();
            renderer.clear(khepri::renderer::Renderer::clear_all);
            renderer.present();
        }

        LOG.info("Shutting down");
        return EXIT_SUCCESS;
    });
    return result ? *result : EXIT_FAILURE;
}