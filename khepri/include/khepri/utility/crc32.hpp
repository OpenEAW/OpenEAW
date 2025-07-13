#pragma once

#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace khepri {
class CRC32
{
private:
    static constexpr std::uint32_t generate_entry(std::uint32_t i)
    {
        std::uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            crc = (crc & 1) ? (0xEDB88320U ^ (crc >> 1)) : (crc >> 1);
        }
        return crc;
    }

    static std::array<std::uint32_t, 256> generate_table() noexcept
    {
        std::array<std::uint32_t, 256> table{};
        for (std::uint32_t i = 0; i < 256; ++i) {
            table[i] = generate_entry(i);
        }
        return table;
    }

    static const std::array<std::uint32_t, 256> CRC_TABLE;

public:
    static std::uint32_t compute(std::string_view data)
    {
        std::uint32_t crc = 0xFFFFFFFFU;
        for (char c : data) {
            crc = CRC_TABLE[(crc ^ static_cast<unsigned char>(c)) & 0xFF] ^ (crc >> 8);
        }
        return crc ^ 0xFFFFFFFFU;
    }
};

// Inline static member for C++17 compatibility — must be defined outside class
inline const std::array<std::uint32_t, 256> CRC32::CRC_TABLE = CRC32::generate_table();

} // namespace khepri
