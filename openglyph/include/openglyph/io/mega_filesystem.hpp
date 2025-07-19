#pragma once

#include <openglyph/io/mega_file.hpp>

#include <cassert>
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace openglyph::io {

/**
 * @brief A virtual file system for accessing Empire at War .MEG archives.
 *
 * The MegaFileSystem class allows reading files stored in one or more Petroglyph-style
 * MegaFile (.MEG) archives used in Star Wars: Empire at War. It searches through multiple
 * MEG files in order and provides access to their contents through a stream interface.
 *
 * @note This class is not thread-safe. External synchronization is required for concurrent use.
 */
class MegaFileSystem final
{
public:
    /**
     * Constructs a new MegaFileSystem instance.
     * this class facilitates loading megaFiles
     * @param data_paths ordered list of paths where to look for assets
     */
    explicit MegaFileSystem(const std::filesystem::path& data_path);

    /**
     * Opens a file from the MegaFileSystem by searching through all MegaFiles.
     *
     * @param data_path The absolute path of the megafile.
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
    std::unique_ptr<khepri::io::Stream> open_file(const std::filesystem::path& path);

private:
    /**
     * @brief Parses the `megafiles.xml` index file to discover referenced MegaFile archives.
     *
     * This function reads a `megafiles.xml` file from the provided stream and extracts the
     * list of `.meg` archive paths referenced in it. These archive paths are then used to
     * locate and load MegaFile archives for asset access.
     *
     * @param data_path The path where the `megafiles.xml` file is located (informational only).
     * @param stream    The input stream containing the contents of the `megafiles.xml` file.
     */
    void parse_index_file(const std::filesystem::path& data_path, khepri::io::Stream& stream);

    std::filesystem::path                  m_data_path;
    std::vector<std::filesystem::path>     m_mega_file_paths;
    std::vector<std::unique_ptr<MegaFile>> m_mega_files;
};
} // namespace openglyph::io