#pragma once

#include <gsl/gsl-lite.hpp>
#include <khepri/io/stream.hpp>

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
    std::unique_ptr<khepri::io::Stream> open_file(const std::filesystem::path&      base_path,
                                                  std::string_view                  name,
                                                  gsl::span<const std::string_view> extensions);

    std::vector<std::filesystem::path> m_data_paths;
};

} // namespace openglyph
