#pragma once

#include <NativeWindow.h>
#include <any>

namespace khepri::renderer::diligent {

// Utility method for extracting a platform-specific native window into a Diligent NativeWindow.
// This is delegated to its own source file to minimize namespace pollution from various
// platform-specific headers.
Diligent::NativeWindow get_native_window(const std::any& window);

} // namespace khepri::renderer::diligent