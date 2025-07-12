#pragma once

#include <openglyph/io/mega_file.hpp>

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

    std::unique_ptr<khepri::io::Stream> open_file(std::filesystem::path& path);

private:
    void parse_index_file(khepri::io::Stream& stream);

    std::vector<std::filesystem::path>     m_data_paths;
    std::vector<std::filesystem::path>     m_mega_file_paths;
    std::vector<std::unique_ptr<MegaFile>> m_mega_files;
};
} // namespace openglyph::io