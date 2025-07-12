#pragma once

#include <openglyph/io/mega_file.hpp>

#include <cassert>
#include <filesystem>
#include <map>
#include <memory>

namespace openglyph::io {
class SubFile;
class MegaFile;
class MegaFileSystem final
{
public:
    /**
     * Constructs a new MegaFileSystem instance.
     * this class facilitates loading megaFiles
     * @param data_paths ordered list of paths where to look for assets
     */
    explicit MegaFileSystem(std::vector<std::filesystem::path> data_paths);

    /**
     * Opens a file from the MegaFileSystem by searching through all MegaFiles.
     *
     * @param path The relative or absolute path of the file to open.
     * @return A unique_ptr to a khepri::io::Stream if the file is found and opened successfully;
     * otherwise nullptr.
     *
     * @note The returned Stream's lifetime is tied to the MegaFile it originates from and may not
     * outlive that MegaFile. Use caution to avoid accessing the Stream after the owning MegaFile
     * has been destroyed.
     *
     * @note This method and the MegaFileSystem are not thread-safe. Concurrent access must be
     * externally synchronized.
     */
    std::unique_ptr<khepri::io::Stream> open_file(std::filesystem::path& path);

private:
    void parse_index_file(khepri::io::Stream& stream);

    std::vector<std::filesystem::path>     m_data_paths;
    std::vector<std::filesystem::path>     m_mega_file_paths;
    std::vector<std::unique_ptr<MegaFile>> m_mega_files;
};
} // namespace openglyph::io