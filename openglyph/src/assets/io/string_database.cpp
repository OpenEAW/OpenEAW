#include <openglyph/assets/io/string_database.hpp>

#include <vector>

namespace openglyph::io {

openglyph::StringDatabase read_string_database(khepri::io::Stream& stream)
{
    std::vector<StringDatabase::StringInfo> string_infos(stream.read_uint32());

    std::uint32_t key_offset   = 0;
    std::uint32_t value_offset = 0;
    for (std::uint32_t i = 0; i < string_infos.size(); ++i) {
        StringDatabase::StringInfo& info = string_infos[i];

        info.crc          = stream.read_uint32();
        info.value_offset = value_offset;
        info.value_length = stream.read_uint32();
        info.key_offset   = key_offset;
        info.key_length   = stream.read_uint32();

        key_offset += info.key_length;
        value_offset += info.value_length;
    }

    std::vector<char16_t> value_buffer(value_offset);
    stream.read(value_buffer.data(), value_buffer.size() * sizeof(char16_t));

    std::vector<char> key_buffer(key_offset);
    stream.read(key_buffer.data(), key_buffer.size());

    return openglyph::StringDatabase(std::move(string_infos), std::move(key_buffer),
                                     std::move(value_buffer));
}

} // namespace openglyph::io
