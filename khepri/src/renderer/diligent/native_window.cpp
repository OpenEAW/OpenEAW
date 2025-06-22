#include "native_window.hpp"

#include <khepri/exceptions.hpp>

#include <cstdint>
#include <tuple>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif __APPLE__
#else
#endif

namespace khepri::renderer::diligent {

Diligent::NativeWindow get_native_window(std::any window)
{
    try {
#ifdef _MSC_VER
        return Diligent::NativeWindow(std::any_cast<HWND>(window));
#elif __APPLE__
        return Diligent::NativeWindow(std::any_cast<NSWindow*>(window));
#else
        const auto native_window = std::any_cast<std::tuple<void*, std::uint32_t>>(window);
        return Diligent::NativeWindow{std::get<1>(native_window), std::get<0>(native_window)};
#endif
    } catch (const std::bad_any_cast&) {
        // Fall-through
    }
    throw ArgumentError();
}

} // namespace khepri::renderer::diligent
