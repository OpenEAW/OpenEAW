#pragma once

#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/io/stream.hpp>

#include <cassert>
#include <filesystem>
#include <map>
#include <memory>

namespace openglyph::io {
class SubFile;
class MegaFile;
class MegaFileSystem
{
public:
    /**
     * Constructs a new MegaFileSystem instance.
     * this class facilitates loading megaFiles
     * @param data_paths ordered list of paths where to look for assets
     */
    MegaFileSystem(std::vector<std::filesystem::path> data_paths);

    std::unique_ptr<SubFile> open_file(std::filesystem::path& path);

private:
    void                                   parse_index_file(khepri::io::Stream& stream);
    std::vector<std::filesystem::path>     m_data_paths;
    std::vector<std::filesystem::path>     m_mega_file_paths;
    std::vector<std::unique_ptr<MegaFile>> m_mega_files;
};

struct SubFileInfo
{
    std::uint32_t crc32;
    std::uint32_t file_index;
    std::uint32_t file_size;
    std::uint32_t file_offset;
    std::uint32_t file_name_index;
};

class MegaFile
{
public:
    MegaFile(const std::filesystem::path& mega_file_path);

    std::unique_ptr<SubFile> open_file(std::filesystem::path& path);

private:
    void extract_metadata(std::vector<std::string>&                filenames,
                          std::vector<openglyph::io::SubFileInfo>& fileinfo);

    khepri::io::File m_megaFile;

    std::vector<std::string> filenames;
    std::vector<SubFileInfo> fileinfo;

    std::vector<std::size_t> file_hashes; // used for lookup.

    std::uint32_t file_name_count = 0;
    std::uint32_t file_info_count = 0;
};
class SubFile : public khepri::io::Stream
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
    std::uint64_t     local_offset;
};
} // namespace openglyph::io