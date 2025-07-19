#pragma once

#include "game_object_type.hpp"

#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/parser/xml_parser.hpp>

#include <map>
#include <memory_resource>
#include <string_view>

namespace openglyph {

/**
 * @brief Loads and stores game object types.
 *
 * The store owns the GameObjectType objects and returns non-owning references.
 */
class GameObjectTypeStore final
{
public:
    /**
     * @brief Constructs a GameObjectTypeStore by loading from XML configuration.
     *
     * The AssetLoader is used to load the @a index_filename configuration file, which is an XML
     * file containing references to other XML files with the actual GameObjectType definitions.
     *
     * All GameObjectType are created from these definitions and stored in this object.
     */
    GameObjectTypeStore(AssetLoader& asset_loader, std::string_view index_filename);

    GameObjectTypeStore(const GameObjectTypeStore&)            = delete;
    GameObjectTypeStore(GameObjectTypeStore&&)                 = delete;
    GameObjectTypeStore& operator=(const GameObjectTypeStore&) = delete;
    GameObjectTypeStore& operator=(GameObjectTypeStore&&)      = delete;
    ~GameObjectTypeStore();

    /**
     * @brief Finds a GameObjectType with a given name.
     *
     * The lifetime of the returned reference is tied to the lifetime of the store.
     *
     * @note the name lookup is case insensitive.
     */
    [[nodiscard]] const GameObjectType* get(std::string_view name) const noexcept;

    /**
     * @brief Finds a GameObjectType by the CRC of its uppercased name.
     *
     * The lifetime of the returned reference is tied to the lifetime of the store.
     *
     * @note in case of types with duplicate CRCs, an arbitrary choice of the duplicates is
     * returned.
     */
    [[nodiscard]] const GameObjectType* get(std::uint32_t crc) const noexcept;

private:
    void read_game_object_types(khepri::io::Stream& stream);

    GameObjectType* read_game_object_type(const XmlParser::Node& node);

    // Copies a string into @a m_memory_resource and returns a view to the copied string
    std::string_view copy_string(std::string_view str);

    // Typically, there are thousands of GameObjectType instances, using a monotonic_buffer_resource
    // to store them significantly speeds up loading and unloading times.
    std::pmr::monotonic_buffer_resource m_memory_resource{std::pmr::get_default_resource()};
    std::pmr::multimap<std::uint32_t, const GameObjectType*> m_game_object_types{
        std::pmr::polymorphic_allocator<std::pair<std::string_view, const GameObjectType*>>(
            &m_memory_resource)};
};

} // namespace openglyph
