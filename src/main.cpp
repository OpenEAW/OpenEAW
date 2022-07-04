#include "version.hpp"

#include <khepri/application/console_logger.hpp>
#include <khepri/log/log.hpp>
#include <khepri/version.hpp>
#include <openglyph/version.hpp>

namespace {
constexpr auto APPLICATION_NAME = "OpenEAW";

constexpr khepri::log::Logger LOG("application");
} // namespace

int main()
{
    khepri::application::ConsoleLogger console_logger;
    LOG.info("Running {} {} (OpenGlyph {}, Khepri {})", APPLICATION_NAME,
             to_string(openeaw::version()), to_string(openglyph::version()),
             to_string(khepri::version()));
}