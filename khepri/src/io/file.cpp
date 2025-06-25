#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/io/stream.hpp>

#include <cassert>
#include <cerrno>
#include <cstring>

namespace khepri::io {

size_t File::read(void* buffer, size_t count)
{
    return std::fread(buffer, 1, count, m_handle);
}

size_t File::write(const void* buffer, size_t count)
{
    return std::fwrite(buffer, 1, count, m_handle);
}

long long File::seek(long long offset, SeekOrigin origin)
{
    int whence = SEEK_SET;
    switch (origin) {
    case SeekOrigin::begin:
        whence = SEEK_SET;
        break;
    case SeekOrigin::current:
        whence = SEEK_CUR;
        break;
    case SeekOrigin::end:
        whence = SEEK_END;
        break;
    default:
        assert(false);
        break;
    }

    if (std::fseek(m_handle, static_cast<long>(offset), whence) != 0) {
        clearerr(m_handle);
        throw Error("Unable to seek file");
    }
    return std::ftell(m_handle);
}

File::File(const Path& path, OpenMode mode) : m_mode(mode)
{
    const char* modestr = nullptr;
    switch (mode) {
    default:
    case OpenMode::read:
        modestr = "rb";
        break;
    case OpenMode::read_write:
        modestr = "w+b";
        break;
    }

    m_handle = std::fopen(path.string().c_str(), modestr);
    if (m_handle == nullptr) {
        throw Error("Unable to open file");
    }
}

File::~File()
{
    (void)std::fclose(m_handle);
}

} // namespace khepri::io
