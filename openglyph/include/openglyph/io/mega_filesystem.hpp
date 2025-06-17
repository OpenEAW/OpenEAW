#pragma once

#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/io/stream.hpp>

#include <cassert>
#include <filesystem>
#include <memory>

namespace openglyph::io {
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
    void parse_index_file(khepri::io::Stream& stream);
    std::vector<std::filesystem::path> m_data_paths;
};

} // namespace openglyph::io