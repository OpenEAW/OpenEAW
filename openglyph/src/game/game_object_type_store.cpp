#include <khepri/exceptions.hpp>
#include <khepri/utility/crc.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/game/game_object_type_store.hpp>
#include <openglyph/parser/parsers.hpp>

#include <algorithm>
#include <variant>

namespace openglyph {

class GameObjectTypeStore::GameObjectTypeReader final
{
public:
    explicit GameObjectTypeReader(std::pmr::monotonic_buffer_resource& memory_resource)
        : m_memory_resource(memory_resource)
        , m_game_object_types(
              std::pmr::polymorphic_allocator<std::pair<std::string_view, const GameObjectType*>>(
                  &m_memory_resource))
    {
    }

    [[nodiscard]] unsigned long read_game_object_types(khepri::io::Stream& stream)
    {
        unsigned long   retry_count = 0;
        const XmlParser parser(stream);
        if (const auto& root = parser.root()) {
            for (const auto& node : root->nodes()) {
                const auto res = read_game_object_type(node);
                if (std::holds_alternative<DuplicateTag>(res)) {
                    // This entry was already read before, ignore it.
                    continue;
                }
                if (std::holds_alternative<RetryTag>(res)) {
                    // This entry cannot be read now and must be retried after all other entries
                    // have been read.
                    ++retry_count;
                    continue;
                }
                assert(std::holds_alternative<GameObjectType*>(res));
                const auto* type = std::get<GameObjectType*>(res);

                m_game_object_types.emplace(khepri::CRC32::calculate(khepri::uppercase(type->name)),
                                            type);
                m_name_index.emplace(type->name, type);
            }
        }
        return retry_count;
    }

    auto finalize() &&
    {
        return std::move(m_game_object_types);
    }

private:
    /// Reads an optional child tag, parsing and storing the value in \a destination.
    /// If the tag is missing or malformed, leaves \a destination untouched.
    template <typename T>
    void read_optional_child(const XmlParser::Node& node, std::string_view child_name,
                             T& destination) const
    {
        if (auto child = node.child(child_name)) {
            if (auto value = child->value(); !value.empty()) {
                destination = parse<T>(value);
            }
        }
    }

    void read_optional_child(const XmlParser::Node& node, std::string_view child_name,
                             std::string_view& destination)
    {
        if (auto child = node.child(child_name)) {
            if (auto value = child->value(); !value.empty()) {
                destination = copy_string(parse<std::string_view>(value));
            }
        }
    }

    struct RetryTag
    {};

    struct DuplicateTag
    {};

    std::variant<GameObjectType*, RetryTag, DuplicateTag>
    read_game_object_type(const XmlParser::Node& node)
    {
        using namespace std::literals;

        const auto name = require_attribute(node, "Name");
        if (m_name_index.contains(name)) {
            // We've already parsed this one before, ignore it
            return DuplicateTag{};
        }

        // Because the memory resource is a monotonic_buffer_resource, tracking the lifetime of the
        // allocated object is not necessary.
        std::pmr::polymorphic_allocator<GameObjectType> allocator(&m_memory_resource);

        GameObjectType* type{nullptr};
        const auto      variant_of = optional_child(node, "Variant_Of_Existing_Type", ""sv);
        if (!variant_of.empty()) {
            const auto it = m_name_index.find(variant_of);
            if (it == m_name_index.end()) {
                // We're a variant of an unknown type. Perhaps it's read later. Notify that we want
                // to retry this entry.
                return RetryTag{};
            }

            // Copy the base type, so we can apply the changes on top
            type = new (allocator.allocate(1)) GameObjectType(*it->second);
        } else {
            type = new (allocator.allocate(1)) GameObjectType();
        }
        type->name = copy_string(name);

        //
        // Read the properties of the type. Take care to not overwrite fields if the XML tag is
        // missing (i.e. support inheritance: derived types can extend base types).
        //

        read_optional_child(node, "Space_Model_Name", type->space_model_name);
        read_optional_child(node, "Scale_Factor", type->scale_factor);
        read_optional_child(node, "In_Background", type->is_in_background);

        if (auto behavior_str = optional_child(node, "Behavior")) {
            for (const auto behavior : khepri::split(*behavior_str, ", \t\r\n")) {
                if (khepri::case_insensitive_equals(behavior, "MARKER")) {
                    type->is_marker = true;
                }
            }
        }
        return type;
    }

    // Copies a string into @a m_memory_resource and returns a view to the copied string
    std::string_view copy_string(std::string_view str)
    {
        std::pmr::polymorphic_allocator<std::string_view::value_type> allocator(&m_memory_resource);
        auto* dest = allocator.allocate(str.size());
        std::copy(str.begin(), str.end(), dest);
        return {dest, str.size()};
    }

    std::pmr::monotonic_buffer_resource&                     m_memory_resource;
    std::pmr::multimap<std::uint32_t, const GameObjectType*> m_game_object_types;
    std::pmr::map<std::string, const GameObjectType*, khepri::CaseInsensitiveLess> m_name_index;
};

GameObjectTypeStore::GameObjectTypeStore(AssetLoader& asset_loader, std::string_view index_filename)
{
    if (auto index_stream = asset_loader.open_config(index_filename)) {
        const XmlParser parser(*index_stream);
        if (const auto& root = parser.root()) {
            GameObjectTypeReader reader(m_memory_resource);
            unsigned long        prev_retry_count = 0;
            unsigned long        retry_count      = 0;
            // Keep parsing all files while we have to retry parsing types. The idea is that some
            // types are variants of other types, but if those haven't been parsed yet, the variant
            // must be retried after the base type has been parsed.
            do {
                retry_count = 0;
                for (const auto& file : root->nodes()) {
                    if (auto config_stream = asset_loader.open_config(file.value())) {
                        retry_count += reader.read_game_object_types(*config_stream);
                    }
                }
                if (retry_count > 0 && prev_retry_count == retry_count) {
                    // Uh oh, we're parsed everything again, but we have to retry just as many
                    // entries as before, so we didn't make any progress. We probably have a
                    // circular reference somewhere, so abort.
                    throw khepri::Error(
                        "Failed to parse GameObjectFiles. Circular reference in variants?");
                }
                prev_retry_count = retry_count;
            } while (retry_count > 0);
            m_game_object_types = std::move(reader).finalize();
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
