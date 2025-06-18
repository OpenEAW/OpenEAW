#include <khepri/io/container_stream.hpp>
#include <khepri/io/exceptions.hpp>

#include <array>
#include <cassert>
#include <type_traits>

namespace khepri::io {
namespace {

// Khepri Container File
constexpr auto MAGIC = std::array<char, 3>{'K', 'C', 'F'};

// File format version
constexpr auto FORMAT_VERSION = 1;

void require(bool condition)
{
    if (!condition) {
        throw InvalidFormatError();
    }
}
} // namespace

ContainerStream::ContainerStream(Stream& underlying_stream, ContentTypeId type_id,
                                 OpenMode open_mode)
    : m_underlying_stream(&underlying_stream), m_open_mode(open_mode)
{
    if (m_open_mode == OpenMode::read) {
        if (!underlying_stream.readable()) {
            throw ArgumentError();
        }

        // Read the file header
        std::remove_const_t<decltype(MAGIC)> file_magic;
        underlying_stream.read(file_magic.data(), file_magic.size());
        require(file_magic == MAGIC);

        const auto version = underlying_stream.read_uint8();
        require(version == FORMAT_VERSION);

        const auto file_type_id = underlying_stream.read_uint32();
        require(type_id == file_type_id);

        const auto flags = underlying_stream.read_uint32();
        require(flags == 0);

        m_content_size = underlying_stream.read_uint32();

        if (underlying_stream.seekable()) {
            m_content_start = underlying_stream.seek(0, SeekOrigin::current);
        }
    } else {
        // When writing, we must be able to seek because we have to write
        // the file size afterwards.
        if (!underlying_stream.seekable() || !underlying_stream.writable()) {
            throw ArgumentError();
        }

        // Write the file header
        underlying_stream.write(MAGIC.data(), MAGIC.size());
        underlying_stream.write_uint8(FORMAT_VERSION);
        underlying_stream.write_uint32(type_id);
        underlying_stream.write_uint32(0); // flags
        underlying_stream.write_uint32(0); // size

        m_content_start = underlying_stream.seek(0, SeekOrigin::current);
    }
}

void ContainerStream::close()
{
    if (m_underlying_stream != nullptr && m_open_mode == OpenMode::write) {
        if (m_content_size <= std::numeric_limits<std::uint32_t>::max()) {
            const auto size_offset =
                m_content_start - static_cast<long long>(sizeof(std::uint32_t));
            m_underlying_stream->seek(size_offset, SeekOrigin::begin);
            m_underlying_stream->write_uint32(static_cast<std::uint32_t>(m_content_size));
        }
        m_underlying_stream = nullptr;
    }
}

std::size_t ContainerStream::read(void* buffer, std::size_t count)
{
    if (!readable()) {
        throw Error("container not opened for reading");
    }

    count           = std::min<std::size_t>(count, m_content_size - m_position);
    const auto read = m_underlying_stream->read(buffer, count);
    m_position += static_cast<long long>(read);
    return read;
}

std::size_t ContainerStream::write(const void* buffer, std::size_t count)
{
    if (!writable()) {
        throw Error("container not opened for writing");
    }

    const auto written = m_underlying_stream->write(buffer, count);
    m_position += static_cast<long long>(written);
    m_content_size = std::max(m_content_size, m_position);
    return written;
}

long long ContainerStream::seek(long long offset, SeekOrigin origin)
{
    if (!seekable()) {
        throw Error("container does not support seeking");
    }

    long long position = 0;
    switch (origin) {
    case SeekOrigin::begin:
        offset   = std::max(0LL, std::min(m_content_size, offset));
        position = m_underlying_stream->seek(m_content_start + offset, SeekOrigin::begin);
        break;

    case SeekOrigin::current:
        position = m_underlying_stream->seek(0, SeekOrigin::current) + offset;
        if (m_open_mode == OpenMode::read) {
            position = std::min(position, m_content_start + m_content_size);
        }
        position = std::max(position, m_content_start);
        break;

    case SeekOrigin::end:
        position =
            (m_open_mode == OpenMode::read)
                ? m_underlying_stream->seek(m_content_start + m_content_size, SeekOrigin::begin)
                : m_underlying_stream->seek(0, SeekOrigin::end);
        break;

    default:
        assert(false);
        break;
    }

    assert(position >= m_content_start);
    return position - m_content_start;
}

} // namespace khepri::io
