#include <khepri/log/log.hpp>

#include <openglyph/parser/parsers.hpp>
#include <openglyph/parser/xml_parser.hpp>
#include <openglyph/renderer/io/material.hpp>

namespace openglyph {
namespace {
enum class PropertyType : std::uint8_t
{
    integer,
    floating,
    vector2,
    vector3,
    vector4,
    matrix,
    texture,
};
}

template <>
struct Parser<PropertyType>
{
    static std::optional<PropertyType> parse(std::string_view str) noexcept
    {
        if (str == "int") {
            return PropertyType::integer;
        }
        if (str == "float") {
            return PropertyType::floating;
        }
        if (str == "float2") {
            return PropertyType::vector2;
        }
        if (str == "float3") {
            return PropertyType::vector3;
        }
        if (str == "float4") {
            return PropertyType::vector4;
        }
        if (str == "matrix") {
            return PropertyType::matrix;
        }
        if (str == "texture") {
            return PropertyType::texture;
        }
        return {};
    }
};

namespace renderer::io {

namespace {
auto load_material(const openglyph::XmlParser::Node& node)
{
    struct MaterialDesc material_desc;
    material_desc.name = require_attribute(node, "Name");
    material_desc.type = optional_attribute(node, "Type", "");

    for (const auto& propnode : node.nodes()) {
        if (propnode.name() == "Shader") {
            material_desc.shader = propnode.value();
        } else if (propnode.name() == "Param") {
            MaterialDesc::Property property;
            property.name = require_attribute(propnode, "Name");
            auto type     = openglyph::parse<PropertyType>(require_attribute(propnode, "Type"));
            switch (type) {
            case PropertyType::integer:
                property.default_value = openglyph::parse<std::int32_t>(propnode.value());
                break;
            case PropertyType::floating:
                property.default_value = openglyph::parse<float>(propnode.value());
                break;
            case PropertyType::vector2:
                property.default_value = openglyph::parse<khepri::Vector2f>(propnode.value());
                break;
            case PropertyType::vector3:
                property.default_value = openglyph::parse<khepri::Vector3f>(propnode.value());
                break;
            case PropertyType::vector4:
                property.default_value = openglyph::parse<khepri::Vector4f>(propnode.value());
                break;
            case PropertyType::matrix:
                property.default_value = openglyph::parse<khepri::Matrixf>(propnode.value());
                break;
            case PropertyType::texture:
                property.default_value = std::string(propnode.value());
                break;
            }
            material_desc.properties.push_back(std::move(property));
        }
    }
    return material_desc;
}

constexpr khepri::log::Logger LOG("renderer");

} // namespace

std::vector<MaterialDesc> load_materials(khepri::io::Stream& xml_stream)
{
    std::vector<MaterialDesc> materials;

    const openglyph::XmlParser xml(xml_stream);
    if (const auto& root = xml.root()) {
        for (const auto& matnode : root->nodes()) {
            try {
                materials.push_back(load_material(matnode));
            } catch (const openglyph::ParseError& e) {
                LOG.error("parse error: {}", e.what());
            }
        }
    }
    return materials;
}

} // namespace renderer::io
} // namespace openglyph