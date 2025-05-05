#pragma once

#include <filesystem>

namespace khepri::application {

/**
 * @brief Get the Current ("Working") Directory
 */
std::filesystem::path get_current_directory();

} // namespace khepri::application