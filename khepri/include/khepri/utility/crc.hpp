#pragma once

#include <cstdint>
#include <string_view>

namespace khepri {

/**
 * Utility class for calculating CRC -- Cyclic Redundancy Checksum, a fast 32-bit hashing method
 * that is not cryptographically secure or particularly resistent against collisions.
 */
class CRC32
{
public:
    static std::uint32_t calculate(std::string_view data);
};

} // namespace khepri