#include <openglyph/io/chunk_reader.hpp>

#include <khepri/io/exceptions.hpp>

#include <cassert>

namespace openglyph::io {

ChunkReader::ChunkReader(khepri::io::Stream& stream) : m_stream(stream)
{
    auto stream_size = m_stream.seek(0, khepri::io::SeekOrigin::end);
    m_stream.seek(0, khepri::io::SeekOrigin::begin);

    // The top-level chunk is "fake": the entire stream
    m_parents.push({0, false, 0, stream_size});

    // Read the first real chunk
    read_next();
}

bool ChunkReader::has_chunk() const noexcept
{
    return !!m_current;
}

ChunkId ChunkReader::id() const
{
    if (!has_chunk()) {
        throw khepri::io::Error("end of chunk reached");
    }
    return m_current->id;
}

bool ChunkReader::has_data() const
{
    if (!has_chunk()) {
        throw khepri::io::Error("end of chunk reached");
    }
    return m_current->data;
}

std::vector<std::uint8_t> ChunkReader::read_data()
{
    if (!has_data()) {
        throw khepri::io::Error("not a data chunk");
    }

    std::vector<std::uint8_t> data(m_current->end - m_current->start);
    if (m_stream.read(data.data(), data.size()) != data.size()) {
        throw khepri::io::InvalidFormatError();
    }
    return data;
}

void ChunkReader::open()
{
    if (has_data()) {
        throw khepri::io::Error("not a parent chunk");
    }

    m_parents.push(*m_current);
    m_current = {};

    read_next();
}

void ChunkReader::close()
{
    if (m_parents.size() == 1) {
        throw khepri::io::Error("no chunk to close");
    }

    m_current = m_parents.top();
    m_parents.pop();

    m_stream.seek(m_current->start, khepri::io::SeekOrigin::begin);
}

void ChunkReader::next()
{
    if (!has_chunk()) {
        throw khepri::io::Error("end of chunk reached");
    }

    // Skip past the current chunk
    auto pos  = m_current->end;
    m_current = {};

    if (pos < m_parents.top().end) {
        m_stream.seek(pos, khepri::io::SeekOrigin::begin);
        read_next();
    }
}

void ChunkReader::read_next()
{
    auto pos = m_stream.seek(0, khepri::io::SeekOrigin::current);
    if (pos + 8 > m_parents.top().end) {
        // The header doesn't fit
        throw khepri::io::InvalidFormatError();
    }

    auto id   = m_stream.read_uint();
    auto size = m_stream.read_uint();
    bool data = ((size & 0x80000000u) == 0);
    pos += 8;
    size = (size & 0x7fffffffu);

    if (pos + size > m_parents.top().end) {
        // The chunk itself doesn't fit
        throw khepri::io::InvalidFormatError();
    }
    m_current = ChunkInfo{id, data, pos, pos + size};
}

MinichunkReader::MinichunkReader(gsl::span<const std::uint8_t> data) : m_data(data)
{
    read_next();
}

ChunkId MinichunkReader::id() const
{
    if (!has_chunk()) {
        throw khepri::io::Error("end of chunk reached");
    }
    return m_current->id;
}

gsl::span<const std::uint8_t> MinichunkReader::read_data() const
{
    if (!has_chunk()) {
        throw khepri::io::Error("end of chunk reached");
    }
    return m_current->data;
}

bool MinichunkReader::has_chunk() const noexcept
{
    return !!m_current;
}

void MinichunkReader::next()
{
    if (!has_chunk()) {
        throw khepri::io::Error("end of chunk reached");
    }

    m_current = {};
    if (m_pos < m_data.size()) {
        read_next();
    }
}

void MinichunkReader::read_next()
{
    if (m_pos + 2 > m_data.size()) {
        // The header doesn't fit
        throw khepri::io::InvalidFormatError();
    }

    auto id   = m_data[m_pos++];
    auto size = m_data[m_pos++];

    if (m_pos + size > m_data.size()) {
        // The chunk itself doesn't fit
        throw khepri::io::InvalidFormatError();
    }
    m_current = ChunkInfo{id, {m_data.data() + m_pos, m_data.data() + m_pos + size}};
    m_pos += size;
}

} // namespace openglyph::io
