#include <khepri/utility/crc.hpp>

#include <array>

namespace khepri {
namespace {

constexpr std::array<std::uint32_t, 256> generate_crc32_table() noexcept
{
    std::array<std::uint32_t, 256> table{};
    for (int i = 0; i < 256; ++i) {
        std::uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            crc = (crc & 1) ? (0xEDB88320U ^ (crc >> 1)) : (crc >> 1);
        }
        table[i] = crc;
    }
    return table;
}

const std::array<std::uint32_t, 256> CRC32_TABLE = generate_crc32_table();
} // namespace

std::uint32_t CRC32::calculate(std::string_view data)
{
    std::uint32_t crc = 0xFFFFFFFFU;
    for (char c : data) {
        crc = CRC32_TABLE[(crc ^ static_cast<unsigned char>(c)) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFU;
}

} // namespace khepri