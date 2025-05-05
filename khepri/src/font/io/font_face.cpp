#include <khepri/font/io/font_face.hpp>

namespace khepri::font::io {

FontFaceDesc load_font_face(khepri::io::Stream& stream)
{
    const auto size = stream.seek(0, khepri::io::SeekOrigin::end);
    stream.seek(0, khepri::io::SeekOrigin::begin);

    std::vector<uint8_t> data(static_cast<std::size_t>(size));
    stream.read(data.data(), data.size());
    return FontFaceDesc{std::move(data)};
}

} // namespace khepri::font::io
