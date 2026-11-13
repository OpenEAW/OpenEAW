#include <openglyph/assets/io/mega_texture.hpp>

namespace openglyph::io {
openglyph::MegaTexture read_mega_texture(khepri::io::Stream& stream)
{
    constexpr std::size_t NAME_SIZE = 64;

    std::array<char, NAME_SIZE + 1> name_buffer;
    name_buffer.back() = '\0';

    openglyph::MegaTexture mega_texture;

    const auto texture_count = stream.read_uint32();
    for (std::uint32_t i = 0; i < texture_count; ++i) {
        stream.read(name_buffer.data(), NAME_SIZE);
        khepri::Rect rect;
        rect.x      = static_cast<long>(stream.read_uint32());
        rect.y      = static_cast<long>(stream.read_uint32());
        rect.width  = stream.read_uint32();
        rect.height = stream.read_uint32();
        stream.read_bool();

        mega_texture.add_texture(name_buffer.data(), rect);
    }

    return mega_texture;
}

} // namespace openglyph::io
