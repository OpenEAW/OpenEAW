#pragma once

#include "quaternion.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace khepri {

template <typename T>
struct std::formatter<BasicVector2<T>>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <class FmtContext>
    FmtContext::iterator format(const khepri::BasicVector2<T>& v, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{{{}, {}}}", v.x, v.y);
    }
};

template <typename T>
struct std::formatter<BasicVector3<T>>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <class FmtContext>
    FmtContext::iterator format(const khepri::BasicVector3<T>& v, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{{{}, {}, {}}}", v.x, v.y, v.z);
    }
};

template <typename T>
struct std::formatter<BasicVector4<T>>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <class FmtContext>
    FmtContext::iterator format(const khepri::BasicVector4<T>& v, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{{{}, {}, {}, {}}}", v.x, v.y, v.z, v.w);
    }
};

template <typename T>
struct std::formatter<BasicQuaternion<T>>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <class FmtContext>
    FmtContext::iterator format(const khepri::BasicQuaternion<T>& q, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{{{}, {}, {}, {}}}", q.x, q.y, q.z, q.w);
    }
};

} // namespace khepri
