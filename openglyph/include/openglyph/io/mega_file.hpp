#pragma once

#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/io/stream.hpp>

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

namespace openglyph::io {

/**
 * @brief Represents a single MegaFile archive.
 *
 * This class provides functionality to open files stored inside
 * a MegaFile archive (.meg). It manages the archive's metadata,
 * including filenames and file info necessary for accessing
 * individual files within the archive.
 *
 * Instances of this class are non-copyable and non-movable.
 */
class MegaFile final
{
public:
    explicit MegaFile(const std::filesystem::path& mega_file_path);
    ~MegaFile() = default;

    MegaFile(const MegaFile&)                = delete;
    MegaFile& operator=(const MegaFile&)     = delete;
    MegaFile(MegaFile&&) noexcept            = delete;
    MegaFile& operator=(MegaFile&&) noexcept = delete;

    std::unique_ptr<khepri::io::Stream> open_file(const std::filesystem::path& path);

private:
    class SubFile;
    struct SubFileInfo
    {
        std::uint32_t crc32;
        std::uint32_t file_index;
        std::uint32_t file_size;
        std::uint32_t file_offset;
        std::uint32_t file_name_index;
    };

    std::tuple<std::vector<std::string>, std::vector<SubFileInfo>> extract_metadata();

    std::unique_ptr<khepri::io::File> m_file;

    std::vector<std::string> m_filenames;
    std::vector<SubFileInfo> m_fileinfo;

    std::vector<std::uint32_t> m_file_crcs; // used for lookup.

    std::uint32_t m_file_name_count = 0;
    std::uint32_t m_file_info_count = 0;
};

} // namespace openglyph::io