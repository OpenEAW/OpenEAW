#pragma once

#include <optional>
#include <string_view>
#include <vector>

namespace openglyph {

class StringDatabase final
{
public:
    struct StringInfo
    {
        std::uint32_t crc;
        std::uint32_t key_offset;
        std::uint32_t key_length;
        std::uint32_t value_offset;
        std::uint32_t value_length;
    };

    StringDatabase();

    StringDatabase(std::vector<StringInfo> strings, std::vector<char> key_buffer,
                   std::vector<char16_t> value_buffer);

    bool is_sorted() const noexcept
    {
        return m_is_sorted;
    }

    std::optional<std::u16string_view> get(std::string_view key) const noexcept;

    std::size_t count() const noexcept
    {
        return m_strings.size();
    }

    std::u16string_view get(std::size_t index) const noexcept;

private:
    std::vector<StringInfo> m_strings;
    std::vector<char>       m_key_buffer;
    std::vector<char16_t>   m_value_buffer;
    bool                    m_is_sorted{true};
};

} // namespace openglyph