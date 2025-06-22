#pragma once

#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/io/stream.hpp>

#include <cassert>
#include <filesystem>
#include <memory>

namespace openglyph::io {
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

private:
    void                               parse_index_file(khepri::io::Stream& stream);
    std::vector<std::filesystem::path> m_data_paths;
    std::vector<std::filesystem::path> m_mega_file_paths;
    std::vector<std::shared_ptr<MegaFile>> m_mega_files;
};

struct SubFileInfo
{
    uint32_t crc32;
    uint32_t file_index;
    uint32_t file_size;
    uint32_t file_offset;
    uint32_t file_name_index;
};
class MegaFile
{
public:
    MegaFile(const std::filesystem::path& mega_file_path);

private:

    /**
     * reads a file info record from the file.
     * @warning can potentialy overflow!
     */
    void read_file_info();
    khepri::io::File         m_megaFile;
    std::vector<std::string> filenames;
    std::vector<SubFileInfo> fileinfo;
    uint32_t                 file_name_count = 0;
    uint32_t                 file_info_count = 0;
};
} // namespace openglyph::io