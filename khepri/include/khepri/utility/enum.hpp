#pragma once

#include <type_traits>

// NOLINTBEGIN(clang-analyzer-optin.core.EnumCastOutOfRange)
template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr T operator~(T a)
{
    return static_cast<T>(~static_cast<typename std::underlying_type_t<T>>(a));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr T operator|(T a, T b)
{
    return static_cast<T>(static_cast<typename std::underlying_type_t<T>>(a) |
                          static_cast<typename std::underlying_type_t<T>>(b));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr T operator&(T a, T b)
{
    return static_cast<T>(static_cast<typename std::underlying_type_t<T>>(a) &
                          static_cast<typename std::underlying_type_t<T>>(b));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr T operator^(T a, T b)
{
    return static_cast<T>(static_cast<typename std::underlying_type_t<T>>(a) ^
                          static_cast<typename std::underlying_type_t<T>>(b));
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr T& operator|=(T& a, T b)
{
    return a = a | b;
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr T& operator&=(T& a, T b)
{
    return a = a & b;
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr T& operator^=(T& a, T b)
{
    return a = a ^ b;
}
// NOLINTEND(clang-analyzer-optin.core.EnumCastOutOfRange)
