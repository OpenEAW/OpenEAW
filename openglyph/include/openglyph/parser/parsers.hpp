#pragma once

#include "exceptions.hpp"

#include <khepri/math/color_srgb.hpp>
#include <khepri/math/matrix.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>
#include <khepri/math/vector4.hpp>
#include <khepri/utility/string.hpp>

#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>

namespace openglyph {
namespace detail {

bool split(std::string_view str, const char* separators, std::string_view* result,
           std::size_t count) noexcept;

} // namespace detail

template <std::size_t N>
struct SplitResult
{
    std::string_view parts[N];
};

/**
 * Splits a string_view into multiple parts, each separated by one of multiple separators
 */
template <std::size_t N>
std::optional<SplitResult<N>> split(std::string_view str, const char* separators) noexcept
{
    SplitResult<N> result;
    if (detail::split(str, separators, result.parts, N)) {
        return result;
    }
    return {};
}

template <typename T, typename = void>
struct Parser
{
    static std::optional<T> parse(std::string_view) = delete;
};

template <typename T>
T parse(std::string_view str)
{
    std::optional<T> val = Parser<T>::parse(khepri::trim(str));
    if (!val) {
        throw ParseError("\"" + std::string(str) + "\" is not a valid value");
    }
    return *val;
}

//
// Specializations of parser<T>
//

template <typename T>
struct Parser<T, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>
{
    static std::optional<T> parse(std::string_view str) noexcept
    {
        T result;

        const auto* end = str.data() + str.size();
        if constexpr (std::is_floating_point_v<T>) {
            // Remove any trailing 'f' suffix
            if (!str.empty() && (str.back() == 'f' || str.back() == 'F')) {
                --end;
            }
        }

        if (auto [ptr, ec] = std::from_chars(str.data(), end, result);
            ptr == end && ec == std::errc()) {
            return result;
        }
        return {};
    }
};

template <>
struct Parser<bool>
{
    static std::optional<bool> parse(std::string_view str) noexcept
    {
        using namespace std::literals;

        if (khepri::case_insensitive_equals(str, "true"sv)) {
            return true;
        }
        if (khepri::case_insensitive_equals(str, "false"sv)) {
            return false;
        }
        return {};
    }
};

template <>
struct Parser<std::string>
{
    static std::optional<std::string> parse(std::string_view str) noexcept
    {
        return std::string(str);
    }
};

template <>
struct Parser<std::string_view>
{
    static std::optional<std::string_view> parse(std::string_view str) noexcept
    {
        return str;
    }
};

template <typename T>
struct Parser<khepri::BasicVector2<T>>
{
    static std::optional<khepri::BasicVector2<T>> parse(std::string_view str) noexcept
    {
        if (auto res = split<2>(str, ",")) {
            return khepri::BasicVector2<T>{openglyph::parse<T>(res->parts[0]),
                                           openglyph::parse<T>(res->parts[1])};
        }
        return {};
    }
};

template <typename T>
struct Parser<khepri::BasicVector3<T>>
{
    static std::optional<khepri::BasicVector3<T>> parse(std::string_view str) noexcept
    {
        if (auto res = split<3>(str, ",")) {
            return khepri::BasicVector3<T>{openglyph::parse<T>(res->parts[0]),
                                           openglyph::parse<T>(res->parts[1]),
                                           openglyph::parse<T>(res->parts[2])};
        }
        return {};
    }
};

template <>
struct Parser<khepri::ColorSRGB>
{
    static std::optional<khepri::ColorSRGB> parse(std::string_view str) noexcept
    {
        using ComponentType = khepri::ColorSRGB::ComponentType;
        if (auto res = split<3>(str, ",")) {
            return khepri::ColorSRGB{openglyph::parse<ComponentType>(res->parts[0]),
                                     openglyph::parse<ComponentType>(res->parts[1]),
                                     openglyph::parse<ComponentType>(res->parts[2])};
        }
        return {};
    }
};

template <typename T>
struct Parser<khepri::BasicVector4<T>>
{
    static std::optional<khepri::BasicVector4<T>> parse(std::string_view str) noexcept
    {
        if (auto res = split<4>(str, ",")) {
            return khepri::BasicVector4{
                openglyph::parse<T>(res->parts[0]), openglyph::parse<T>(res->parts[1]),
                openglyph::parse<T>(res->parts[2]), openglyph::parse<T>(res->parts[3])};
        }
        return {};
    }
};

template <typename T>
struct Parser<khepri::BasicMatrix<T>>
{
    static std::optional<khepri::BasicMatrix<T>> parse(std::string_view str) noexcept
    {
        if (auto res = split<16>(str, ",")) {
            return khepri::BasicMatrix<T>{
                openglyph::parse<T>(res->parts[0]),  openglyph::parse<T>(res->parts[1]),
                openglyph::parse<T>(res->parts[2]),  openglyph::parse<T>(res->parts[3]),
                openglyph::parse<T>(res->parts[4]),  openglyph::parse<T>(res->parts[5]),
                openglyph::parse<T>(res->parts[6]),  openglyph::parse<T>(res->parts[7]),
                openglyph::parse<T>(res->parts[8]),  openglyph::parse<T>(res->parts[9]),
                openglyph::parse<T>(res->parts[10]), openglyph::parse<T>(res->parts[11]),
                openglyph::parse<T>(res->parts[12]), openglyph::parse<T>(res->parts[13]),
                openglyph::parse<T>(res->parts[14]), openglyph::parse<T>(res->parts[15])};
        }
        return {};
    }
};

} // namespace openglyph
