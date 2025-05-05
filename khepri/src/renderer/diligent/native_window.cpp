#include "native_window.hpp"

#include <khepri/exceptions.hpp>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace khepri::renderer::diligent {

Diligent::NativeWindow get_native_window(std::any window)
{
    try {
#ifdef _MSC_VER
        return Diligent::NativeWindow(std::any_cast<HWND>(window));
#endif
    } catch (const std::bad_any_cast&) {
        // Fall-through
    }
    throw ArgumentError();
}

} // namespace khepri::renderer::diligent
