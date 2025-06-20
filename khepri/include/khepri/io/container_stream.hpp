#pragma once

#include "stream.hpp"

namespace khepri::io {

/**
 *\brief A container stream
 *
 * Container streams are regular streams that contain opaque data but include an ID as
 * a check on the type of content and various flags that allow for e.g. compression
 * and encryption of the contained data.
 */
class ContainerStream final : public Stream
{
public:
    /// Mode to open files in
    enum class OpenMode
    {
        /// Open files in read-only mode
        read,

        /// Open files in read-write mode
        write,
    };

    /// Type describing content IDs
    using ContentTypeId = std::uint32_t;

    /**
     * \brief Constructs a contrainer_stream
     * \param[in] underlying_stream the underlying stream to use for reading or writing.
     * \param[in] type_id the id of the type of content this stream will or should contain.
     * \param[in] open_mode whether the container should be opened for reading or writing.
     *
     * \throw invalid_format if \a underlying stream does not contain a valid container with the
     *                       specified content type.
     * \throw argument_error if the underlying stream does not support \a open_mode.
     */
    ContainerStream(Stream& underlying_stream, ContentTypeId type_id, OpenMode open_mode);
    ~ContainerStream() noexcept override = default;

    ContainerStream(const ContainerStream&)            = delete;
    ContainerStream& operator=(const ContainerStream&) = delete;
    ContainerStream(ContainerStream&&)                 = delete;
    ContainerStream& operator=(ContainerStream&&)      = delete;

    /**
     * \brief Closes the container.
     * This must be called when writing a container or the resulting container is invalid.
     */
    void close();

    /// \see stream::readable
    [[nodiscard]] bool readable() const noexcept override
    {
        return m_underlying_stream != nullptr && m_open_mode == OpenMode::read;
    }

    /// \see stream::writable
    [[nodiscard]] bool writable() const noexcept override
    {
        return m_underlying_stream != nullptr && m_open_mode == OpenMode::write;
    }

    /// \see stream::seekable
    [[nodiscard]] bool seekable() const noexcept override
    {
        return m_underlying_stream != nullptr && m_underlying_stream->seekable();
    }

    /// \see stream::read
    std::size_t read(void* buffer, std::size_t count) override;

    /// \see stream::write
    std::size_t write(const void* buffer, std::size_t count) override;

    /// \see stream::seek
    long long seek(long long offset, SeekOrigin origin) override;

private:
    Stream*   m_underlying_stream;
    OpenMode  m_open_mode;
    long long m_position{0};

    // For reading
    long long m_content_start{0};
    long long m_content_size{0};
};

} // namespace khepri::io
