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
    auto type = new (allocator.allocate(1)) GameObjectType();

    type->name             = copy_string(require_attribute(node, "Name"));
    type->space_model_name = copy_string(optional_child(node, "Space_Model_Name", ""sv));
    type->scale_factor     = optional_child(node, "Scale_Factor", 1.0);
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
    XmlParser parser(stream);
    if (const auto& root = parser.root()) {
        for (const auto& node : root->nodes()) {
            auto type = read_game_object_type(node);
            m_game_object_types.emplace(type->name, type);
        }
    }
}

GameObjectTypeStore::GameObjectTypeStore(AssetLoader& asset_loader, std::string_view index_filename)
{
    if (auto index_stream = asset_loader.open_config(index_filename)) {
        XmlParser parser(*index_stream);
        if (const auto& root = parser.root()) {
            for (const auto& file : root->nodes()) {
                if (auto config_stream = asset_loader.open_config(file.value())) {
                    read_game_object_types(*config_stream);
                }
            }
        }
    }
}

GameObjectTypeStore::~GameObjectTypeStore() = default;

} // namespace openglyph
