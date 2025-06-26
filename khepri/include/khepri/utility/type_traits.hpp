#pragma once

#include <type_traits>

namespace khepri {
namespace detail {
template <typename To>
constexpr std::true_type  is_narrowing_conversion_helper(To);
constexpr std::false_type is_narrowing_conversion_helper(...);
} // namespace detail

/**
 * \brief Determines if a conversion between two types is a narrowing conversion.
 *
 * IsNarrowingConversion::value is a constant expression with value 1 if the conversion from \a
 * From to \a To is a narrowing conversion, 0 otherwise.
 *
 * \a From and \a To must be arithmetic types.
 */

template <typename From, typename To, typename = void, typename = void>
struct IsNarrowingConversion : std::true_type
{};

template <typename From, typename To>
struct IsNarrowingConversion<
    From, To, std::enable_if_t<std::is_arithmetic_v<From> && std::is_arithmetic_v<To>>,
    std::enable_if_t<decltype(detail::is_narrowing_conversion_helper<To>(
        {std::declval<From>()}))::value>> : std::false_type
{};

template <typename From, typename To>
// NOLINTNEXTLINE(readability-identifier-naming) -- snake_case to stay in line with std code
inline constexpr bool is_narrowing_conversion_v = IsNarrowingConversion<From, To>::value;

} // namespace khepri