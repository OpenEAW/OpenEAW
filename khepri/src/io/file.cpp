#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
namespace khepri::io {

size_t File::read(void* buffer, size_t count)
{
    if (!m_stream.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(count))) {
        // read may fail due to EOF or error, but we can still get the number of bytes read
        return static_cast<size_t>(m_stream.gcount());
    }
    return count;
}

size_t File::write(const void* buffer, size_t count)
{
    m_stream.write(reinterpret_cast<const char*>(buffer), static_cast<std::streamsize>(count));
    if (!m_stream) {
        // Write failed, so return 0 bytes written
        return 0;
    }
    return count;
}

long long File::seek(long long offset, SeekOrigin origin)
{
    std::ios_base::seekdir dir;

    switch (origin) {
    case SeekOrigin::begin:
        dir = std::ios_base::beg;
        break;
    case SeekOrigin::current:
        dir = std::ios_base::cur;
        break;
    case SeekOrigin::end:
        dir = std::ios_base::end;
        break;
    default:
        throw Error("Invalid seek origin");
    }

    m_stream.clear(); // Clear any eof/fail bits before seeking

    // Seek input position
    m_stream.seekg(offset, dir);
    if (!m_stream) {
        throw Error("Failed to seek input position");
    }

    // Seek output position (if writable)
    if (writable()) {
        m_stream.seekp(offset, dir);
        if (!m_stream) {
            throw Error("Failed to seek output position");
        }
    }

    // Return the new position relative to beginning
    auto pos = m_stream.tellg();
    if (pos == -1) {
        throw Error("Failed to get file position");
    }

    return static_cast<long long>(pos);
}

File::File(const std::filesystem::path& path, OpenMode mode)
    : m_mode(mode)
    , m_stream(path, (mode == OpenMode::read)
                         ? (std::ios::in | std::ios::binary)
                         : (std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc))
{
    if (!m_stream.is_open()) {
        throw Error("Unable to open file");
    }
}

File::~File()
{
    m_stream.close();
}
} // namespace khepri::io
