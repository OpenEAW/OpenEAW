#pragma once

#include <string>

namespace khepri::io {

enum class SeekOrigin
{
    begin,   ///< Seek from the beginning of the file.
    current, ///< Seek from the current position in the file.
    end,     ///< Seek from the end of the file.
};

/// A base class for streams
class Stream
{
public:
    virtual ~Stream() noexcept = default;

    Stream(const Stream&) = delete;
    Stream(Stream&&)      = delete;
    Stream& operator=(const Stream&) = delete;
    Stream& operator=(Stream&&) = delete;

    /// Checks if the stream is readable
    [[nodiscard]] virtual bool readable() const noexcept = 0;

    /// Checks if the stream is writable
    [[nodiscard]] virtual bool writable() const noexcept = 0;

    /// Checks if the stream is seekable
    [[nodiscard]] virtual bool seekable() const noexcept = 0;

    /**
     * \brief Reads data from the stream at the current position.
     *
     * \param[in] buffer the memory to write the read data to.
     * \param[in] count the number of consecutive bytes to read.
     *
     * \return the number of bytes read and stored in \a buffer.
     */
    virtual size_t read(void* buffer, size_t count) = 0;

    /**
     * \brief Writes data to the stream at the current position.
     *
     * \param[in] buffer pointer to the data to write.
     * \param[in] count the number of consecutive bytes to write.
     *
     * \return the number of bytes from \a buffer written to the stream.
     */
    virtual size_t write(const void* buffer, size_t count) = 0;

    /**
     * \brief Changes the file position
     *
     * \param[in] offset offset by how much to change the file position
     * \param[in] origin the origin from whence to change the position
     *
     * \return the new file position, from the start of the file
     */
    virtual long long seek(long long offset, SeekOrigin origin) = 0;

    /// Reads a boolean (one byte) from the stream
    bool read_bool()
    {
        return read_uint8() != 0;
    }

    /// Reads a 16-bit signed little-endian integer from the stream
    int16_t read_int16();

    /// Reads a 32-bit signed little-endian integer from the stream
    int32_t read_int32();

    /// Reads a 64-bit signed little-endian integer from the stream
    int64_t read_int64();

    /// Reads a 32-bit little-endian IEEE 754 floating-point number from the stream
    float read_float();

    /// Reads an unsigned byte from the stream
    uint8_t read_uint8();

    /// Reads a 16-bit unsigned little-endian integer from the stream
    uint16_t read_uint16();

    /// Reads a 32-bit unsigned little-endian integer from the stream
    uint32_t read_uint32();

    /// Reads a 64-bit unsigned little-endian integer from the stream
    uint64_t read_uint64();

    /**
     * \brief Reads a string from the stream.
     *
     * This first reads the length of the string as if via #read_uint16(), followed
     * by that many bytes as the contents of the string.
     */
    std::string read_string();

    /// Writes a boolean (one byte) to the stream
    void write_bool(bool b)
    {
        write_uint8(b ? 1 : 0);
    }

    /// Writes a 16-bit signed little-endian integer to the stream
    void write_int8(int8_t i8);

    /// Writes a 16-bit signed little-endian integer to the stream
    void write_int16(int16_t i16);

    /// Writes a 32-bit signed little-endian integer to the stream
    void write_int32(int32_t i32);

    /// Writes a 64-bit signed little-endian integer to the stream
    void write_int64(int64_t i64);

    /// Writes a byte/uint8_t to the stream
    void write_uint8(uint8_t u8);

    /// Writes a 16-bit unsigned little-endian integer to the stream
    void write_uint16(uint16_t u16);

    /// Writes a 32-bit unsigned little-endian integer to the stream
    void write_uint32(uint32_t u32);

    /// Writes a 32-bit unsigned little-endian integer to the stream
    void write_uint64(uint64_t u64);

    /// Writes a 32-bit little-endian IEEE-754 floating-point number to the stream
    void write_float(float f);

    /**
     * \brief Writes a string to the stream.
     *
     * This first writes the length of the string as if via #write_uint16(), followed
     * by that many bytes as the contents of the string.
     *
     * \note The length of \a s must not exceed 65535.
     */
    void write_string(const std::string& s);

protected:
    Stream() = default;
};

} // namespace khepri::io
