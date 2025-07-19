#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace khepri {
/**
 * @brief Utility class for computing CRC32 checksums.
 *
 * This class provides a static method to compute a 32-bit cyclic redundancy check (CRC32)
 * checksum over a block of data.
 */
class CRC32
{
public:
    /**
     * @brief Computes the CRC32 checksum for the given data.
     *
     * @param data The input data as a string view.
     * @return The computed CRC32 checksum as a 32-bit unsigned integer.
     */
    static std::uint32_t compute(std::string_view data);
};

} // namespace khepri
