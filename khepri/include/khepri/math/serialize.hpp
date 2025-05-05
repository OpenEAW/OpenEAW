#pragma once

#include "color_rgba.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

#include <khepri/io/serialize.hpp>

namespace khepri::io {

/// Specialization of #khepri::io::SerializeTraits for #khepri::BasicVector2
template <typename T>
struct SerializeTraits<BasicVector2<T>>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const BasicVector2<T>& value)
    {
        s.write(value.x);
        s.write(value.y);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static BasicVector2<T> deserialize(Deserializer& d)
    {
        BasicVector2<T> v;
        v.x = d.read<BasicVector2<T>::ComponentType>();
        v.y = d.read<BasicVector2<T>::ComponentType>();
        return v;
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::BasicVector3
template <typename T>
struct SerializeTraits<BasicVector3<T>>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const BasicVector3<T>& value)
    {
        s.write(value.x);
        s.write(value.y);
        s.write(value.z);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static BasicVector3<T> deserialize(Deserializer& d)
    {
        BasicVector3<T> v;
        v.x = d.read<BasicVector3<T>::ComponentType>();
        v.y = d.read<BasicVector3<T>::ComponentType>();
        v.z = d.read<BasicVector3<T>::ComponentType>();
        return v;
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::BasicVector4
template <typename T>
struct SerializeTraits<BasicVector4<T>>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const BasicVector4<T>& value)
    {
        s.write(value.x);
        s.write(value.y);
        s.write(value.z);
        s.write(value.w);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static BasicVector4<T> deserialize(Deserializer& d)
    {
        BasicVector4<T> v;
        v.x = d.read<BasicVector4<T>::ComponentType>();
        v.y = d.read<BasicVector4<T>::ComponentType>();
        v.z = d.read<BasicVector4<T>::ComponentType>();
        v.w = d.read<BasicVector4<T>::ComponentType>();
        return v;
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::ColorRGBA
template <>
struct SerializeTraits<ColorRGBA>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const ColorRGBA& value)
    {
        s.write(value.r);
        s.write(value.g);
        s.write(value.b);
        s.write(value.a);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static ColorRGBA deserialize(Deserializer& d)
    {
        ColorRGBA c;
        c.r = d.read<ColorRGBA::ComponentType>();
        c.g = d.read<ColorRGBA::ComponentType>();
        c.b = d.read<ColorRGBA::ComponentType>();
        c.a = d.read<ColorRGBA::ComponentType>();
        return c;
    }
};

} // namespace khepri::io
