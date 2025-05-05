#include <khepri/renderer/io/shader.hpp>

namespace khepri::renderer::io {

ShaderDesc load_shader(khepri::io::Stream& stream)
{
    const auto size = stream.seek(0, khepri::io::SeekOrigin::end);
    stream.seek(0, khepri::io::SeekOrigin::begin);

    std::vector<uint8_t> data(static_cast<std::size_t>(size));
    stream.read(data.data(), data.size());
    return ShaderDesc{std::move(data)};
}

} // namespace khepri::renderer::io
