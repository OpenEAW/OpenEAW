#include "version.hpp"

#include <khepri/application/application.hpp>
#include <khepri/log/log.hpp>
#include <khepri/renderer/diligent/renderer.hpp>
#include <khepri/version.hpp>
#include <openglyph/version.hpp>

namespace {
constexpr auto APPLICATION_NAME = "OpenEAW";

constexpr khepri::log::Logger LOG("application");
} // namespace

class MainApplication : public khepri::application::Application
{
public:
    MainApplication() : khepri::application::Application(APPLICATION_NAME) {}

private:
    void do_run(khepri::application::Window& window, const std::filesystem::path& working_path) override
    {
        khepri::renderer::diligent::Renderer renderer(
            Diligent::Win32NativeWindow(window.native_handle()));
        window.add_size_listener([&] { renderer.render_size(window.render_size()); });
        renderer.render_size(window.render_size());

        while (!window.should_close()) {
            khepri::application::Window::poll_events();
            renderer.clear(khepri::renderer::Renderer::clear_all);
            renderer.present();
        }
    }
};

int main()
{
    MainApplication application;

    LOG.info("Running {} {} (OpenGlyph {}, Khepri {})", APPLICATION_NAME,
             to_string(openeaw::version()), to_string(openglyph::version()),
             to_string(khepri::version()));

    application.run();
}