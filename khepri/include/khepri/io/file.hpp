#pragma once

#include "stream.hpp"

#include <gsl/gsl-lite.hpp>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>

namespace khepri::io {

/// Modes for dealing with files
enum class OpenMode : std::uint8_t
{
    read,       /// Opens an existing file for reading.
    read_write, /// Creates a new file for reading and writing.
};

/// A file-based stream
class File : public Stream
{
public:
    /// Opens a file for reading or reading and writing.
    /// \throws khepri::io::error if the file cannot be opened.
    File(const std::filesystem::path& path, OpenMode mode);
    ~File() override;

    File(const File&)            = delete;
    File& operator=(const File&) = delete;

    File(File&& other) noexcept;
    File& operator=(File&& other) noexcept;

    /// \see stream::readable
    [[nodiscard]] bool readable() const noexcept override
    {
        return true;
    }

    /// \see stream::writable
    [[nodiscard]] bool writable() const noexcept override
    {
        return m_mode == OpenMode::read_write;
    }

    /// \see stream::seekable
    [[nodiscard]] bool seekable() const noexcept override
    {
        return true;
    }

    /// \see stream::read
    size_t read(void* buffer, size_t count) override;

    /// \see stream::write
    size_t write(const void* buffer, size_t count) override;

    /// \see stream::seek
    long long seek(long long offset, io::SeekOrigin origin) override;

private:
    std::fstream m_stream;
    OpenMode     m_mode;
};

} // namespace khepri::io
