#pragma once

#include <khepri/io/stream.hpp>

#include <gsl/gsl-lite.hpp>
#include <openglyph/io/mega_filesystem.hpp>

#include <filesystem>
#include <memory>
#include <string_view>
#include <utility>

namespace openglyph {

/**
 * Locates and loads assets according to Glyph's asset layout.
 *
 * It can look in multiple paths, loading a requested asset in the first path its found.
 *
 * It is also flexible in finding asset filenames. Each asset type has a list of extensions that the
 * AssetLoader will attempt to look through. For instance, when requesting texture "W_BLANK", it
 * may look for "W_BLANK", "W_BLANK.DDS" and "W_BLANK.TGA".
 */
class AssetLoader
{
public:
    /**
     * Constructs a new AssetLoader.
     *
     * @param data_paths ordered list of paths where to look for assets
     */
    explicit AssetLoader(std::vector<std::filesystem::path> data_paths);

    /**
     * Opens a configuration asset
     */
    std::unique_ptr<khepri::io::Stream> open_config(std::string_view name);

    /**
     * Opens a texture asset
     */
    std::unique_ptr<khepri::io::Stream> open_texture(std::string_view name);

    /**
     * Opens a model asset
     */
    std::unique_ptr<khepri::io::Stream> open_model(std::string_view name);

    /**
     * Opens a shader asset
     */
    std::unique_ptr<khepri::io::Stream> open_shader(std::string_view name);

    /**
     * Opens a map asset
     */
    std::unique_ptr<khepri::io::Stream> open_map(std::string_view name);

private:
    /**
     * @brief Provides access to asset files from a single data source (physical or MegaFile).
     *
     * Each AssetLayer represents one data path.
     *
     * The class abstracts the file-loading logic by attempting to open files in the following
     * order:
     * 1. From the physical file system rooted at `data_path`.
     * 2. From a MegaFile archive using `MegaFileSystem`.
     *
     */
    class AssetLayer
    {
    public:
        explicit AssetLayer(const std::filesystem::path& data_path);

        /**
         * @brief Attempts to open a file from this asset layer.
         *
         * The method tries to open the specified file in the following order:
         * 1. Directly from the physical filesystem under the base data path.
         * 2. From physical files with each of the given extensions appended.
         * 3. From the MegaFile archive (if available) using the base path.
         * 4. From the MegaFile archive (if available) with each of the given extensions appended.
         *
         * @param path The relative path of the file to open.
         * @param extensions A list of alternative file extensions to try if the base file is not
         * found or an extension is not provided in the path.
         * @return A unique pointer to a Stream representing the opened file, or nullptr if no file
         * could be opened.
         */
        std::unique_ptr<khepri::io::Stream> open_file(const std::filesystem::path&      path,
                                                      gsl::span<const std::string_view> extensions);

    private:
        std::filesystem::path m_data_path{};
        io::MegaFileSystem    m_megafs;
    };

    std::unique_ptr<khepri::io::Stream> open_file(const std::filesystem::path&      base_path,
                                                  std::string_view                  name,
                                                  gsl::span<const std::string_view> extensions);

    std::vector<AssetLayer> m_asset_layers;
};

} // namespace openglyph
