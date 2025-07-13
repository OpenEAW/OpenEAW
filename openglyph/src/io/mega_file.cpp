#include <khepri/utility/crc32.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/io/mega_file.hpp>
namespace openglyph::io {

class MegaFile::SubFile : public khepri::io::Stream
{
public:
    SubFile(const SubFileInfo info, khepri::io::File* file);

    SubFile(const SubFile&)            = delete;
    SubFile(SubFile&&)                 = delete;
    SubFile& operator=(const SubFile&) = delete;
    SubFile& operator=(SubFile&&)      = delete;

    /// \see stream::readable
    [[nodiscard]] bool readable() const noexcept override
    {
        return true;
    }

    /// \see stream::writable
    [[nodiscard]] bool writable() const noexcept override
    {
        return false;
    }

    /// \see stream::seekable
    [[nodiscard]] bool seekable() const noexcept override
    {
        return true;
    }

    /**
     * \brief Reads data from the stream at the current position.
     *
     * \param[in] buffer the memory to write the read data to.
     * \param[in] count the number of consecutive bytes to read.
     *
     * \return the number of bytes read and stored in \a buffer.
     */
    virtual size_t read(void* buffer, size_t count) override;

    /**
     * \brief Writes data to the stream at the current position (not supported for subfiles).
     *
     * \param[in] buffer pointer to the data to write.
     * \param[in] count the number of consecutive bytes to write.
     *
     * \return the number of bytes from \a buffer written to the stream.
     */
    virtual size_t write(const void* buffer, size_t count) override;

    /**
     * \brief Changes the file position
     *
     * \param[in] offset offset by how much to change the file position
     * \param[in] origin the origin from whence to change the position
     *
     * \return the new file position, from the start of the file
     */
    virtual long long seek(long long offset, khepri::io::SeekOrigin origin) override;

private:
    const SubFileInfo info;
    khepri::io::File* mega_file;
    std::uint64_t     local_read_offset;
};

MegaFile::MegaFile(const std::filesystem::path& mega_file_path)
    : m_file(std::make_unique<khepri::io::File>(mega_file_path, khepri::io::OpenMode::read))
{
    std::tie(m_filenames, m_fileinfo) = extract_metadata();

    // calculate hashes for this session
    for (const SubFileInfo& info : m_fileinfo) {
        m_file_crcs.push_back(info.crc32);
    }
}

std::unique_ptr<khepri::io::Stream> MegaFile::open_file(std::filesystem::path& path)
{
    std::uint32_t crc = khepri::CRC32::compute(khepri::uppercase(path.string()));

    auto it = std::lower_bound(m_file_crcs.begin(), m_file_crcs.end(), crc);

    if (it != m_file_crcs.end() && *it == crc) {
        std::size_t index = static_cast<std::size_t>(std::distance(m_file_crcs.begin(), it));
        return std::make_unique<SubFile>(m_fileinfo[index], m_file.get());
    }

    return nullptr;
}
std::tuple<std::vector<std::string>, std::vector<openglyph::io::SubFileInfo>>
MegaFile::extract_metadata()
{
    uint32_t file_name_count = m_file->read_uint32();
    uint32_t file_info_count = m_file->read_uint32();

    std::vector<std::string> filenames;
    filenames.reserve(file_name_count);

    for (size_t i = 0; i < file_name_count; ++i) {
        filenames.emplace_back(m_file->read_string());
    }

    std::vector<openglyph::io::SubFileInfo> fileinfo;
    fileinfo.reserve(file_info_count);

    for (size_t i = 0; i < file_info_count; ++i) {
        openglyph::io::SubFileInfo info = {m_file->read_uint32(), m_file->read_uint32(),
                                           m_file->read_uint32(), m_file->read_uint32(),
                                           m_file->read_uint32()};
        fileinfo.emplace_back(info);
    }

    return {std::move(filenames), std::move(fileinfo)};
}

MegaFile::SubFile::SubFile(const SubFileInfo info, khepri::io::File* file)
    : info(info), mega_file(file)
{
}

size_t MegaFile::SubFile::read(void* buffer, size_t count)
{
    // Another file could have read from the current file, so update the read head.
    mega_file->seek(info.file_offset + local_read_offset, khepri::io::SeekOrigin::begin);

    // Count could be larger then the file, prevent reading past the edge of the sub file
    count = std::min(count, info.file_size - local_read_offset);

    local_read_offset += count;
    return mega_file->read(buffer, count);
}

size_t MegaFile::SubFile::write(const void* buffer, size_t count)
{
    throw khepri::io::NotSupportedError();
}

long long MegaFile::SubFile::seek(long long offset, khepri::io::SeekOrigin origin)
{
    std::int64_t new_local_read_offset = static_cast<std::int64_t>(local_read_offset);
    switch (origin) {
    case khepri::io::SeekOrigin::begin:
        new_local_read_offset = offset;
        break;
    case khepri::io::SeekOrigin::current:
        new_local_read_offset += offset;
        break;
    case khepri::io::SeekOrigin::end:
        new_local_read_offset = info.file_size + offset;
        break;
    default:
        break;
    }

    new_local_read_offset =
        std::clamp(new_local_read_offset, 0LL, static_cast<std::int64_t>(info.file_size));

    // Assign back to uint64_t after clamping
    local_read_offset = static_cast<uint64_t>(new_local_read_offset);

    return local_read_offset;
}

} // namespace openglyph::io
