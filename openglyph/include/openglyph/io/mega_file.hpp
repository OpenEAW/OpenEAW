#pragma once

#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/io/stream.hpp>

namespace openglyph::io {
struct SubFileInfo
{
    std::uint32_t crc32;
    std::uint32_t file_index;
    std::uint32_t file_size;
    std::uint32_t file_offset;
    std::uint32_t file_name_index;
};

class MegaFile final
{
public:
    explicit MegaFile(const std::filesystem::path& mega_file_path);

    MegaFile(const MegaFile&)                = delete;
    MegaFile& operator=(const MegaFile&)     = delete;
    MegaFile(MegaFile&&) noexcept            = delete;
    MegaFile& operator=(MegaFile&&) noexcept = delete;

    std::unique_ptr<khepri::io::Stream> open_file(std::filesystem::path& path);

private:
    class SubFile;

    void extract_metadata(std::vector<std::string>&                filenames,
                          std::vector<openglyph::io::SubFileInfo>& fileinfo);

    std::unique_ptr<khepri::io::File> m_megaFile;

    std::vector<std::string> filenames;
    std::vector<SubFileInfo> fileinfo;

    std::vector<std::size_t> file_hashes; // used for lookup.

    std::uint32_t file_name_count = 0;
    std::uint32_t file_info_count = 0;
};
} // namespace openglyph::io