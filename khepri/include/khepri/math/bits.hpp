#pragma once

#include <cstdint>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace khepri {

/// Counts the number of bits in a 16-bit integer
inline unsigned int bitcount(std::uint16_t value)
{
#if defined(_MSC_VER)
    return static_cast<unsigned int>(__popcnt16(value));
#elif defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned int>(__builtin_popcount(value));
#else
#error "the platform does not support bitcount()"
#endif
}

/// Counts the number of bits in a 32-bit integer
inline unsigned int bitcount(std::uint32_t value)
{
#if defined(_MSC_VER)
    return static_cast<unsigned int>(__popcnt(value));
#elif defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned int>(__builtin_popcountl(value));
#else
#error "the platform does not support bitcount()"
#endif
}

/// Counts the number of bits in a 64-bit integer
inline unsigned int bitcount(std::uint64_t value)
{
#if defined(_MSC_VER)
    return static_cast<unsigned int>(__popcnt64(value));
#elif defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned int>(__builtin_popcountll(value));
#else
#error "the platform does not support bitcount()"
#endif
}

/// Returns the first power of two equal to or greater than the value
inline constexpr std::uint32_t ceil_power_of_two(std::uint32_t value)
{
    // From Bit Twiddling Hacks:
    // "It works by copying the highest set bit to all of the lower bits, and then adding one,
    //  which results in carries that set all of the lower bits to 0 and one bit beyond the highest
    //  set bit to 1. If the original number was a power of 2, then the decrement will reduce it to
    //  one less, so that we round up to the same original value."
    //
    // Using a bitscan instrinsic might be an alternative, but that may take several cycles so some
    // performance measurement would have to prove it's worth it.
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return value + 1;
}

} // namespace khepri