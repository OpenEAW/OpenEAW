#include <khepri/utility/string.hpp>

#include <openglyph/io/mega_file.hpp>

namespace openglyph::io {

class MegaFile::SubFile : public khepri::io::Stream
{
public:
    SubFile(const SubFileInfo info, khepri::io::File* p_file);

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
    khepri::io::File* p_megaFile;
    std::uint64_t     local_read_offset;
};

MegaFile::MegaFile(const std::filesystem::path& mega_file_path)
    : m_megaFile(std::make_unique<khepri::io::File>(mega_file_path, khepri::io::OpenMode::read))
{
    extract_metadata(filenames, fileinfo);

    // calculate hashes for this session
    for (const SubFileInfo& info : fileinfo) {
        std::size_t hash = std::hash<std::string>{}(filenames[info.file_name_index]);
        file_hashes.push_back(hash);
    }
}

std::unique_ptr<khepri::io::Stream> MegaFile::open_file(std::filesystem::path& path)
{
    std::size_t hash = std::hash<std::string>{}(khepri::uppercase(path.string()));
    auto        it   = std::find(file_hashes.begin(), file_hashes.end(), hash);

    if (it != file_hashes.end()) {
        std::size_t index = static_cast<std::size_t>(std::distance(file_hashes.begin(), it));
        return std::make_unique<SubFile>(fileinfo[index], m_megaFile.get());
    }
    throw khepri::io::FileNotFoundError();
}
void MegaFile::extract_metadata(std::vector<std::string>&                filenames,
                                std::vector<openglyph::io::SubFileInfo>& fileinfo)
{
    file_name_count = m_megaFile->read_uint32();
    file_info_count = m_megaFile->read_uint32();

    for (size_t i = 0; i < file_name_count; i++) {
        filenames.emplace_back(m_megaFile->read_string());
    }

    for (size_t i = 0; i < file_name_count; i++) {
        SubFileInfo info = {m_megaFile->read_uint32(), m_megaFile->read_uint32(),
                            m_megaFile->read_uint32(), m_megaFile->read_uint32(),
                            m_megaFile->read_uint32()};

        fileinfo.emplace_back(info);
    }
}

MegaFile::SubFile::SubFile(const SubFileInfo info, khepri::io::File* p_file)
    : info(info), p_megaFile(p_file)
{
}

size_t MegaFile::SubFile::read(void* buffer, size_t count)
{
    // Another file could have read from the current file, so update the read head.
    p_megaFile->seek(info.file_offset + local_read_offset, khepri::io::SeekOrigin::begin);

    // Count could be larger then the file, prevent reading past the edge of the sub file
    if (local_read_offset + count > info.file_size) {
        count = info.file_size - local_read_offset;
    }
    local_read_offset += count;
    return p_megaFile->read(buffer, count);
}

size_t MegaFile::SubFile::write(const void* buffer, size_t count)
{
    throw khepri::io::Error("writing is not supported");
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
