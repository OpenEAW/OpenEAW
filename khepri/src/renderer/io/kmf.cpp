#include <khepri/io/container_stream.hpp>
#include <khepri/io/exceptions.hpp>
#include <khepri/io/serialize.hpp>
#include <khepri/log/log.hpp>
#include <khepri/math/serialize.hpp>
#include <khepri/renderer/exceptions.hpp>
#include <khepri/renderer/io/kmf.hpp>
#include <khepri/renderer/io/serialize.hpp>

namespace khepri::renderer::io {

namespace {
constexpr log::Logger LOG("kmf");

constexpr khepri::io::ContainerStream::ContentTypeId CONTENT_ID_KMF = 0x3ea69ae9;

void require(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}
} // namespace

ModelDesc load_kmf(khepri::io::Stream& stream)
{
    if (!stream.readable() || !stream.seekable()) {
        throw ArgumentError();
    }

    khepri::io::ContainerStream container(stream, CONTENT_ID_KMF,
                                          khepri::io::ContainerStream::OpenMode::read);

    auto size = container.seek(0, khepri::io::SeekOrigin::end);
    container.seek(0, khepri::io::SeekOrigin::begin);
    std::vector<std::uint8_t> buffer(size);
    if (container.read(buffer.data(), buffer.size()) != buffer.size()) {
        throw khepri::io::Error("unable to read stream");
    }

    try {
        khepri::io::Deserializer deserializer(buffer);
        return deserializer.read<ModelDesc>();
    } catch (const khepri::io::Error&) {
        throw khepri::io::InvalidFormatError();
    }
}

void write_kmf(const ModelDesc& model, khepri::io::Stream& stream)
{
    if (!stream.writable() || !stream.seekable()) {
        throw ArgumentError();
    }

    khepri::io::ContainerStream container(stream, CONTENT_ID_KMF,
                                          khepri::io::ContainerStream::OpenMode::write);
    khepri::io::Serializer      serializer;
    serializer.write(model);
    auto data = serializer.data();
    if (container.write(data.data(), data.size()) != data.size()) {
        throw khepri::io::Error("unable to write stream");
    }
    container.close();
}

} // namespace khepri::renderer::io
