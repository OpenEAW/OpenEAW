#pragma once

#include <khepri/io/serialize.hpp>
#include <khepri/renderer/model_desc.hpp>

namespace khepri::io {

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::MeshDesc::Vertex
template <>
struct SerializeTraits<renderer::MeshDesc::Vertex>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::MeshDesc::Vertex& value)
    {
        s.write(value.position);
        s.write(value.normal);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::MeshDesc::Vertex deserialize(Deserializer& d)
    {
        renderer::MeshDesc::Vertex vertex;
        vertex.position = d.read<Vector3f>();
        vertex.normal   = d.read<Vector3f>();
        return vertex;
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::MeshDesc
template <>
struct SerializeTraits<renderer::MeshDesc>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::MeshDesc& value)
    {
        s.write(value.vertices);
        s.write(value.indices);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::MeshDesc deserialize(Deserializer& d)
    {
        auto vertices = d.read<std::vector<renderer::MeshDesc::Vertex>>();
        auto indices  = d.read<std::vector<renderer::MeshDesc::Index>>();
        return renderer::MeshDesc{std::move(vertices), std::move(indices)};
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::ModelDesc
template <>
struct SerializeTraits<renderer::ModelDesc>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::ModelDesc& value)
    {
        s.write(value.meshes());
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::ModelDesc deserialize(Deserializer& d)
    {
        auto meshes = d.read<std::vector<renderer::MeshDesc>>();
        return renderer::ModelDesc(std::move(meshes));
    }
};

} // namespace khepri::io
