#pragma once

#include <filesystem>

namespace khepri::application {

/**
 * @brief Get the Current ("Working") Directory
 *
 * @throw khepri::Error if the current directory cannot be determined.
 */
std::filesystem::path get_current_directory();

} // namespace khepri::application