#include <khepri/utility/crc.hpp>
#include <khepri/utility/string.hpp>

#include <gsl/gsl-lite.hpp>
#include <openglyph/game/behaviors/marker_behavior.hpp>
#include <openglyph/game/game_object_type_store.hpp>
#include <openglyph/parser/parsers.hpp>

#include <algorithm>

namespace openglyph {
namespace {

template <typename T>
T optional_child(const XmlParser::Node& node, std::string_view child_name, const T& default_value)
{
    if (auto child = node.child(child_name)) {
        if (auto value = child->value(); !value.empty()) {
            return parse<T>(value);
        }
    }
    return default_value;
}
} // namespace

GameObjectType* GameObjectTypeStore::read_game_object_type(const XmlParser::Node& node)
{
    using namespace std::literals;

    // Because the memory resource is a monotonic_buffer_resource, tracking the lifetime of the
    // allocated object is not necessary.
    std::pmr::polymorphic_allocator<GameObjectType> allocator(&m_memory_resource);
    auto* type = gsl::owner<GameObjectType*>(new (allocator.allocate(1)) GameObjectType());

    type->name             = copy_string(require_attribute(node, "Name"));
    type->space_model_name = copy_string(optional_child(node, "Space_Model_Name", ""sv));
    type->scale_factor     = optional_child(node, "Scale_Factor", 1.0);
    type->is_in_background = optional_child(node, "In_Background", false);

    if (auto behavior_str = optional_child(node, "Behavior")) {
        for (const auto behavior : khepri::split(*behavior_str, ", \t\r\n")) {
            if (khepri::case_insensitive_equals(behavior, "MARKER")) {
                type->is_marker = true;
            }
        }
    }
    return type;
}

std::string_view GameObjectTypeStore::copy_string(std::string_view str)
{
    std::pmr::polymorphic_allocator<std::string_view::value_type> allocator(&m_memory_resource);
    auto* dest = allocator.allocate(str.size());
    std::copy(str.begin(), str.end(), dest);
    return {dest, str.size()};
}

void GameObjectTypeStore::read_game_object_types(khepri::io::Stream& stream)
{
    const XmlParser parser(stream);
    if (const auto& root = parser.root()) {
        for (const auto& node : root->nodes()) {
            const auto* type = read_game_object_type(node);
            m_game_object_types.emplace(khepri::CRC32::calculate(khepri::uppercase(type->name)),
                                        type);
        }
    }
}

GameObjectTypeStore::GameObjectTypeStore(AssetLoader& asset_loader, std::string_view index_filename)
{
    if (auto index_stream = asset_loader.open_config(index_filename)) {
        const XmlParser parser(*index_stream);
        if (const auto& root = parser.root()) {
            for (const auto& file : root->nodes()) {
                if (auto config_stream = asset_loader.open_config(file.value())) {
                    read_game_object_types(*config_stream);
                }
            }
        }
    }
}

const GameObjectType* GameObjectTypeStore::get(std::string_view name) const noexcept
{
    const auto [first, last] =
        m_game_object_types.equal_range(khepri::CRC32::calculate(khepri::uppercase(name)));
    for (auto it = first; it != last; ++it) {
        if (khepri::case_insensitive_equals(it->second->name, name)) {
            return it->second;
        }
    }
    return nullptr;
}

const GameObjectType* GameObjectTypeStore::get(std::uint32_t crc) const noexcept
{
    const auto it = m_game_object_types.find(crc);
    return (it != m_game_object_types.end()) ? it->second : nullptr;
}

GameObjectTypeStore::~GameObjectTypeStore() = default;

} // namespace openglyph
