#pragma once

#include <gsl/gsl-lite.hpp>
#include <khepri/io/stream.hpp>

#include <cstdint>
#include <optional>
#include <stack>
#include <vector>

namespace openglyph::io {

using ChunkId = std::uint32_t;

/**
 * A chunk_reader reads the chunked file format from a stream
 */
class ChunkReader final
{
public:
    /**
     * Constructs a chunk reader.
     *
     * \param[in] stream the underlying stream.
     *
     * \note The caller must ensure that @a stream is kept alive while this object is alive.
     */
    explicit ChunkReader(khepri::io::Stream& stream);

    /**
     * Returns the ID of the current chunk
     * \throws khepri::io::error if #has_chunks() is false
     */
    ChunkId id() const;

    /**
     * Does the current chunk contain data or chunks?
     * \throws khepri::io::error if #has_chunks() is false
     */
    bool has_data() const;

    /**
     * Reads the current chunk's data.
     *
     * \throws khepri::io::error if #has_chunks() is false, #has_data() is false or an I/O error
     * occured.
     */
    std::vector<std::uint8_t> read_data();

    /**
     * Has the end of the current level's chunks been reached?
     */
    bool has_chunk() const noexcept;

    /**
     * Advances the reader to the next chunk.
     * \throws khepri::io::error if #has_chunks() is false or an I/O error occured.
     * \note This invalidates any returned streams or child chunk_readers.
     */
    void next();

    /**
     * Opens the current chunk.
     *
     * \throws khepri::io::error if #has_chunks() is false, #data() is true or an I/O error occured.
     */
    void open();

    /**
     * Closes the current chunks and moves back to the parent.
     *
     * \throws khepri::io::error if the current chunk is the top-level chunk.
     */
    void close();

private:
    void read_next();

    struct ChunkInfo
    {
        ChunkId   id;
        bool      data;
        long long start;
        long long end;
    };

    khepri::io::Stream&      m_stream;
    std::optional<ChunkInfo> m_current;
    std::stack<ChunkInfo>    m_parents;
};

/**
 * A minichunk_reader reads minichunks from a data blob
 */
class MinichunkReader final
{
public:
    /**
     * Constructs a mini-chunk reader.
     *
     * \param[in] data the underlying data blob.
     *
     * \note The caller must ensure that @a data is kept alive while this object is alive.
     */
    explicit MinichunkReader(gsl::span<const std::uint8_t> data);

    /**
     * Returns the ID of the current mini-chunk
     * \throws khepri::io::error if #has_chunks() is true
     */
    ChunkId id() const;

    /**
     * Reads the current mini-chunk's data.
     * \throws khepri::io::error if #has_chunks() is true
     */
    gsl::span<const std::uint8_t> read_data() const;

    /**
     * Has the end of the mini-chunks been reached?
     */
    bool has_chunk() const noexcept;

    /**
     * Advances the reader to the next mini-chunk.
     * \throws khepri::io::error if #has_chunks() is false or an I/O error occured.
     */
    void next();

private:
    void read_next();

    struct ChunkInfo
    {
        ChunkId                       id;
        gsl::span<const std::uint8_t> data;
    };

    gsl::span<const std::uint8_t> m_data;
    size_t                        m_pos{0};
    std::optional<ChunkInfo>      m_current;
};

} // namespace openglyph::io
