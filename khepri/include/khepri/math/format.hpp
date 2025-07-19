#pragma once

#include "quaternion.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

#include <fmt/format.h>

namespace khepri {

template <typename T>
struct fmt::formatter<BasicVector2<T>>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const khepri::BasicVector2<T>& v, format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{{{}, {}}}", v.x, v.y);
    }
};

template <typename T>
struct fmt::formatter<BasicVector3<T>>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const khepri::BasicVector3<T>& v, format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{{{}, {}, {}}}", v.x, v.y, v.z);
    }
};

template <typename T>
struct fmt::formatter<BasicVector4<T>>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const khepri::BasicVector4<T>& v, format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{{{}, {}, {}, {}}}", v.x, v.y, v.z, v.w);
    }
};

template <typename T>
struct fmt::formatter<BasicQuaternion<T>>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const khepri::BasicQuaternion<T>& q, format_context& ctx) const
    {
        return fmt::format_to(ctx.out(), "{{{}, {}, {}, {}}}", q.x, q.y, q.z, q.w);
    }
};

} // namespace khepri
