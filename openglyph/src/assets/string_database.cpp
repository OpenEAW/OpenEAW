#include <khepri/exceptions.hpp>
#include <khepri/utility/crc.hpp>

#include <openglyph/assets/string_database.hpp>

#include <algorithm>
#include <cassert>

namespace openglyph {
namespace {
bool crc_less(const StringDatabase::StringInfo& a, const StringDatabase::StringInfo& b)
{
    return a.crc < b.crc;
}

bool is_valid(const std::vector<StringDatabase::StringInfo>& strings,
              const std::vector<char>& key_buffer, const std::vector<char16_t>& value_buffer)
{
    for (const auto& string_info : strings) {
        if (string_info.key_offset + string_info.key_length > key_buffer.size()) {
            return false;
        }
        if (string_info.value_offset + string_info.value_length > value_buffer.size()) {
            return false;
        }
    }
    return true;
}
} // namespace

StringDatabase::StringDatabase() = default;

StringDatabase::StringDatabase(std::vector<StringInfo> strings, std::vector<char> key_buffer,
                               std::vector<char16_t> value_buffer)
    : m_strings(std::move(strings))
    , m_value_buffer(std::move(value_buffer))
    , m_key_buffer(std::move(key_buffer))
    , m_is_sorted(std::is_sorted(m_strings.begin(), m_strings.end(), crc_less))
{
    assert(is_valid(strings, key_buffer, value_buffer));
}

std::optional<std::u16string_view> StringDatabase::get(std::string_view key) const noexcept
{
    if (!m_is_sorted) {
        return {};
    }

    const auto key_crc = khepri::CRC32::calculate(key);
    const auto it      = std::lower_bound(m_strings.begin(), m_strings.end(), key_crc,
                                          [&](const StringInfo& string_info, std::uint32_t key_crc) {
                                         return string_info.crc < key_crc;
                                     });
    if (it == m_strings.end() || it->crc != key_crc) {
        return {};
    }

    assert(it->key_offset + it->key_length <= m_key_buffer.size());
    assert(it->value_offset + it->value_length <= m_value_buffer.size());
    return std::u16string_view{&m_value_buffer[it->value_offset], it->value_length};
}

std::u16string_view StringDatabase::get(std::size_t index) const noexcept
{
    assert(index < m_strings.size());
    const auto& string_info = m_strings[index];
    assert(string_info.key_offset + string_info.key_length <= m_key_buffer.size());
    assert(string_info.value_offset + string_info.value_length <= m_value_buffer.size());
    return std::u16string_view{&m_value_buffer[string_info.value_offset], string_info.value_length};
}

} // namespace openglyph