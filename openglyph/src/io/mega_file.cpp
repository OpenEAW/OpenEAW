#include <khepri/math/math.hpp>
#include <khepri/utility/crc.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/io/mega_file.hpp>

#include <iostream>
namespace openglyph::io {

namespace {
void verify(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}
} // namespace

/**
 * @brief this class represents a subfile inside the megafile archive.
 *
 * @note This instance may not outlive its parent megafile archive.
 */
class MegaFile::SubFile : public khepri::io::Stream
{
public:
    SubFile(const SubFileInfo& info, khepri::io::File* file);

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

    /// \see stream::read
    virtual size_t read(void* buffer, size_t count) override;

    /// \see stream::write
    virtual size_t write(const void* buffer, size_t count) override;

    /// \see stream::seek
    virtual long long seek(long long offset, khepri::io::SeekOrigin origin) override;

private:
    const MegaFile::SubFileInfo info;
    khepri::io::File*           m_mega_file;
    std::uint64_t               m_local_read_offset;
};

MegaFile::MegaFile(const std::filesystem::path& mega_file_path)
    : m_file(std::make_unique<khepri::io::File>(mega_file_path, khepri::io::OpenMode::read))
{
    std::tie(m_filenames, m_fileinfo) = extract_metadata();
}

std::unique_ptr<khepri::io::Stream> MegaFile::open_file(const std::filesystem::path& path)
{
    std::uint32_t crc            = khepri::CRC32::calculate(khepri::uppercase(path.string()));
    std::string   uppercase_path = khepri::uppercase(path.string());

    auto it = std::lower_bound(
        m_fileinfo.begin(), m_fileinfo.end(), crc,
        [](const SubFileInfo& info, std::uint32_t value) { return info.crc32 < value; });

    while (it != m_fileinfo.end() && it->crc32 == crc) {
        std::string_view file_path = m_filenames[it->file_name_index];
        if (file_path == uppercase_path) {
            return std::make_unique<SubFile>(*it, m_file.get());
        }
        ++it; // linear scearch until we see a different CRC32 from the matched one.
    }

    return nullptr;
}
std::tuple<std::vector<std::string>, std::vector<MegaFile::SubFileInfo>>
MegaFile::extract_metadata()
{
    uint32_t file_name_count = m_file->read_uint32();
    uint32_t file_info_count = m_file->read_uint32();

    std::vector<std::string> filenames;
    filenames.reserve(file_name_count);

    for (size_t i = 0; i < file_name_count; ++i) {
        filenames.push_back(m_file->read_string());
    }

    std::vector<SubFileInfo> fileinfo;
    fileinfo.reserve(file_info_count);

    for (size_t i = 0; i < file_info_count; ++i) {
        MegaFile::SubFileInfo info = {m_file->read_uint32(), m_file->read_uint32(),
                                      m_file->read_uint32(), m_file->read_uint32(),
                                      m_file->read_uint32()};
        fileinfo.push_back(info);
    }

    verify(std::is_sorted(fileinfo.begin(), fileinfo.end(),
                          [](const auto& a, const auto& b) { return a.crc32 < b.crc32; }));

    return {std::move(filenames), std::move(fileinfo)};
}

MegaFile::SubFile::SubFile(const SubFileInfo& info, khepri::io::File* file)
    : info(info), m_mega_file(file)
{
}

size_t MegaFile::SubFile::read(void* buffer, size_t count)
{
    // Another file could have read from the current file, so update the read head.
    m_mega_file->seek(info.file_offset + m_local_read_offset, khepri::io::SeekOrigin::begin);

    // Count could be larger then the file, prevent reading past the edge of the sub file
    count = std::min(count, info.file_size - m_local_read_offset);

    size_t actual_read = m_mega_file->read(buffer, count);
    m_local_read_offset += actual_read;
    return actual_read;
}

size_t MegaFile::SubFile::write(const void* buffer, size_t count)
{
    throw khepri::io::NotSupportedError();
}

long long MegaFile::SubFile::seek(long long offset, khepri::io::SeekOrigin origin)
{
    std::int64_t new_local_read_offset = static_cast<std::int64_t>(m_local_read_offset);
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

    new_local_read_offset = khepri::clamp(new_local_read_offset, static_cast<std::int64_t>(0),
                                          static_cast<std::int64_t>(info.file_size));

    // Assign back to uint64_t after clamping
    m_local_read_offset = static_cast<uint64_t>(new_local_read_offset);

    return m_local_read_offset;
}

} // namespace openglyph::io
