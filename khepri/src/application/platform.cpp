#include <khepri/application/platform.hpp>

namespace khepri::application {

Platform get_current_platform()
{
#if defined(_WIN32) || defined(_WIN64)
    return Platform::windows;
#elif defined(__linux__)
    return Platform::linux;
#else
#error "Unsupported platform"
#endif
}

} // namespace khepri::application
