#include <gsl/gsl-lite.hpp>
#include <khepri/io/exceptions.hpp>
#include <khepri/io/serialize.hpp>
#include <khepri/math/serialize.hpp>
#include <khepri/utility/string.hpp>
#include <openglyph/io/chunk_reader.hpp>
#include <openglyph/renderer/io/model.hpp>

#include <algorithm>
#include <charconv>
#include <tuple>

using Model = openglyph::renderer::Model;

namespace openglyph::io {
namespace {
enum class ChunkId
{
    mesh                 = 0x400,
    mesh_name            = 0x401,
    mesh_info            = 0x402,
    submesh              = 0x10000,
    submesh_info         = 0x10001,
    submesh_indices      = 0x10004,
    submesh_vertices_v1  = 0x10005,
    submesh_vertices_v2  = 0x10007,
    shader_info          = 0x10100,
    shader_name          = 0x10101,
    shader_param_int     = 0x10102,
    shader_param_float   = 0x10103,
    shader_param_float3  = 0x10104,
    shader_param_texture = 0x10105,
    shader_param_float4  = 0x10106,
};

struct VertexV1 : Model::Vertex
{};

struct VertexV2 : Model::Vertex
{};
} // namespace
} // namespace openglyph::io

namespace khepri::io {
/// Specialization of #khepri::io::SerializeTraits for \c VertexV1
template <>
struct SerializeTraits<openglyph::io::VertexV1>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const openglyph::io::VertexV1& value)
    {
        s.write(value.position);
        s.write(value.normal);
        for (int i = 0; i < 4; ++i) {
            s.write(value.uv[i]);
        }
        s.write(value.tangent);
        s.write(value.binormal);
        s.write(value.color);
        for (int i = 0; i < 4; ++i) {
            s.write(std::uint32_t{0});
        }
        for (int i = 0; i < 4; ++i) {
            s.write(0.0f);
        }
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static openglyph::io::VertexV1 deserialize(Deserializer& d)
    {
        openglyph::io::VertexV1 v;
        v.position = d.read<khepri::Vector3f>();
        v.normal   = d.read<khepri::Vector3f>();
        for (int i = 0; i < 4; ++i) {
            v.uv[i] = d.read<khepri::Vector2f>();
        }
        v.tangent  = d.read<khepri::Vector3f>();
        v.binormal = d.read<khepri::Vector3f>();
        v.color    = d.read<khepri::ColorRGBA>();
        for (int i = 0; i < 4; ++i) {
            d.read<std::uint32_t>();
        }
        for (int i = 0; i < 4; ++i) {
            d.read<float>();
        }
        return v;
    }
};

/// Specialization of #khepri::io::SerializeTraits for \c vertex_v2
template <>
struct SerializeTraits<openglyph::io::VertexV2>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const openglyph::io::VertexV2& value)
    {
        s.write(value.position);
        s.write(value.normal);
        for (int i = 0; i < 4; ++i) {
            s.write(value.uv[i]);
        }
        s.write(value.tangent);
        s.write(value.binormal);
        s.write(value.color);
        s.write(khepri::Vector4f{0, 0, 0, 0});
        for (int i = 0; i < 4; ++i) {
            s.write(std::uint32_t{0});
        }
        for (int i = 0; i < 4; ++i) {
            s.write(0.0f);
        }
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static openglyph::io::VertexV2 deserialize(Deserializer& d)
    {
        openglyph::io::VertexV2 v;
        v.position = d.read<khepri::Vector3f>();
        v.normal   = d.read<khepri::Vector3f>();
        for (int i = 0; i < 4; ++i) {
            v.uv[i] = d.read<khepri::Vector2f>();
        }
        v.tangent  = d.read<khepri::Vector3f>();
        v.binormal = d.read<khepri::Vector3f>();
        v.color    = d.read<khepri::ColorRGBA>();
        d.read<khepri::Vector4f>();
        for (int i = 0; i < 4; ++i) {
            d.read<std::uint32_t>();
        }
        for (int i = 0; i < 4; ++i) {
            d.read<float>();
        }
        return v;
    }
};
} // namespace khepri::io

namespace openglyph::io {
namespace {
void verify(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}

std::string as_string(gsl::span<const uint8_t> data)
{
    auto end = std::find(data.begin(), data.end(), 0);
    return {data.begin(), end};
}

// Parses a mesh name into it's name, LOD and ALT levels
auto parse_mesh_name(std::string_view name)
{
    int lod = 0, alt = 0;

    auto alt_ofs = name.find("_ALT");
    if (alt_ofs != std::string_view::npos) {
        int   value;
        auto* end      = name.data() + name.size();
        auto [ptr, ec] = std::from_chars(name.data() + alt_ofs + 4, end, value);
        if (ptr == end && ec == std::errc()) {
            name = name.substr(0, alt_ofs);
            alt  = value;
        }
    }

    auto lod_ofs = name.find("_LOD");
    if (lod_ofs != std::string_view::npos) {
        int   value;
        auto* end      = name.data() + name.size();
        auto [ptr, ec] = std::from_chars(name.data() + lod_ofs + 4, end, value);
        if (ptr == end && ec == std::errc()) {
            name = name.substr(0, lod_ofs);
            lod  = value;
        }
    }

    return std::make_tuple(std::string(name), lod, alt);
}

auto read_submesh(ChunkReader& reader)
{
    std::vector<Model::Vertex> vertices;
    std::vector<Model::Index>  indices;

    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case ChunkId::submesh_info: {
            verify(reader.has_data());
            const auto               data = reader.read_data();
            khepri::io::Deserializer d(data);
            vertices.resize(d.read<std::uint32_t>());
            indices.resize(d.read<std::uint32_t>() * 3);
            break;
        }

        case ChunkId::submesh_vertices_v1: {
            verify(reader.has_data());
            const auto               data = reader.read_data();
            khepri::io::Deserializer d(data);
            std::generate(vertices.begin(), vertices.end(),
                          [&]() -> Model::Vertex { return d.read<VertexV1>(); });
            break;
        }

        case ChunkId::submesh_vertices_v2: {
            verify(reader.has_data());
            const auto               data = reader.read_data();
            khepri::io::Deserializer d(data);
            std::generate(vertices.begin(), vertices.end(),
                          [&]() -> Model::Vertex { return d.read<VertexV2>(); });
            break;
        }

        case ChunkId::submesh_indices: {
            verify(reader.has_data());
            const auto               data = reader.read_data();
            khepri::io::Deserializer d(data);
            std::generate(indices.begin(), indices.end(), [&] { return d.read<Model::Index>(); });
            break;
        }
        }
    }
    return std::make_tuple(std::move(vertices), std::move(indices));
}

template <typename T>
T read_material_param_value(gsl::span<const std::uint8_t> data)
{
    return khepri::io::Deserializer(data).read<T>();
}

template <>
std::string read_material_param_value<std::string>(gsl::span<const std::uint8_t> data)
{
    return as_string(data);
}

template <typename T>
Model::Material::Param read_material_param(gsl::span<const std::uint8_t> data)
{
    Model::Material::Param param;
    MinichunkReader        reader(data);
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case 1:
            param.name = as_string(reader.read_data());
            break;
        case 2:
            param.value = read_material_param_value<T>(reader.read_data());
            break;
        }
    }
    return param;
}

auto read_shader_info(ChunkReader& reader)
{
    std::string                         name;
    std::vector<Model::Material::Param> params;

    for (; reader.has_chunk(); reader.next()) {
        auto id = reader.id();
        switch (id) {
        case ChunkId::shader_name:
            verify(reader.has_data());
            name = as_string(reader.read_data());
            break;
        case ChunkId::shader_param_int:
            verify(reader.has_data());
            params.push_back(read_material_param<std::int32_t>(reader.read_data()));
            break;
        case ChunkId::shader_param_float:
            verify(reader.has_data());
            params.push_back(read_material_param<float>(reader.read_data()));
            break;
        case ChunkId::shader_param_float3:
            verify(reader.has_data());
            params.push_back(read_material_param<khepri::Vector3f>(reader.read_data()));
            break;
        case ChunkId::shader_param_float4:
            verify(reader.has_data());
            params.push_back(read_material_param<khepri::Vector4f>(reader.read_data()));
            break;
        case ChunkId::shader_param_texture:
            verify(reader.has_data());
            params.push_back(read_material_param<std::string>(reader.read_data()));
            break;
        }
    }
    return std::make_tuple(std::move(name), std::move(params));
}

Model::Mesh read_mesh(ChunkReader& reader)
{
    Model::Mesh mesh;
    int         submesh_idx = 0;
    int         shader_idx  = 0;

    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case ChunkId::mesh_name:
            verify(reader.has_data());
            std::tie(mesh.name, mesh.lod, mesh.alt) =
                parse_mesh_name(as_string(reader.read_data()));
            break;

        case ChunkId::mesh_info: {
            verify(reader.has_data());
            const auto               data = reader.read_data();
            khepri::io::Deserializer d(data);
            mesh.materials.resize(d.read<std::uint32_t>());
            d.read<khepri::Vector3f>();
            d.read<khepri::Vector3f>();
            d.read<std::uint32_t>();
            mesh.visible = (d.read<std::uint32_t>() == 0);
            break;
        }

        case ChunkId::submesh: {
            verify(!reader.has_data());
            verify(submesh_idx < mesh.materials.size());
            auto& mat = mesh.materials[submesh_idx];
            reader.open();
            std::tie(mat.vertices, mat.indices) = read_submesh(reader);
            reader.close();
            submesh_idx++;
            break;
        }

        case ChunkId::shader_info:
            verify(!reader.has_data());
            verify(shader_idx < mesh.materials.size());
            auto& mat = mesh.materials[shader_idx];
            reader.open();
            std::tie(mat.name, mat.params) = read_shader_info(reader);
            reader.close();
            shader_idx++;
            break;
        }
    }
    return mesh;
}
} // namespace

Model read_model(khepri::io::Stream& stream)
{
    Model       model;
    ChunkReader reader(stream);
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case ChunkId::mesh:
            verify(!reader.has_data());
            reader.open();
            model.meshes.push_back(read_mesh(reader));
            reader.close();
            break;
        }
    }

    return model;
}

} // namespace openglyph::io
