#pragma once

#include "string.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace khepri {

/**
 * \brief Cache that owns the cached items.
 *
 * This cache can be used to avoid loading expensive-to-create objects multiple times by loading
 * them on-demand. An injected 'loader' callback is used to load uncached items on-demand.
 *
 * \tparam Value the type of object being cached. This cache owns the objects.
 * \tparam Key the type of key used to identify the object.
 * \tparam KeyView a non-owning view of the Key type for lookups.
 */
template <typename Value, typename Key = std::string, typename KeyView = std::string_view>
class OwningCache final
{
public:
    /// Callback type for loading items on-demand
    using Loader = std::function<std::unique_ptr<Value>(const KeyView&)>;

    /**
     * Constructs the cache.
     *
     * \param item_loader the callback used to load items on-demand.
     *
     * \note the cache takes ownership of loaded items
     */
    explicit OwningCache(Loader item_loader) : m_item_loader(std::move(item_loader)) {}

    /**
     * Returns a callable that returns the same thing as calling #get on this cache
     */
    auto as_loader()
    {
        return [this](std::string_view id) { return this->get(id); };
    }

    /**
     * Finds or loads an object with the specified id.
     *
     * If the object does not exist in this cache, the cache's loader is called and the result is
     * cached.
     *
     * \param id the ID of the object to retrieve or load.
     *
     * \return a non-owning pointer to the cached object, or nullptr if the loader returned nullptr.
     *
     * \note the returned pointer remains valid until the cache is destroyed or cleared.
     */
    Value* get(const KeyView& id)
    {
        auto it = m_items.find(id);
        if (it == m_items.end()) {
            auto item = m_item_loader(id);
            if (!item) {
                return nullptr;
            }
            it = m_items.emplace(Key{id}, std::move(item)).first;
        }
        return it->second.get();
    }

    /**
     * Clears all items from the cache.
     *
     * \note any pointer obtained via #get are invalidated.
     */
    void clear()
    {
        m_items.clear();
    }

private:
    Loader m_item_loader;

    std::map<Key, std::unique_ptr<Value>, CaseInsensitiveLess> m_items;
};

} // namespace khepri