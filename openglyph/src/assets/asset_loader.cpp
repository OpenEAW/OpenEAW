#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/log/log.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/io/mega_filesystem.hpp>

#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace openglyph {
namespace {
constexpr khepri::log::Logger LOG("assets");

fs::path base_path()
{
    return "Data";
}
} // namespace

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
class AssetLoader::AssetLayer
{
public:
    explicit AssetLayer(const std::filesystem::path& data_path);

    ~AssetLayer() = default;

    AssetLayer(const AssetLayer&)            = delete;
    AssetLayer& operator=(const AssetLayer&) = delete;

    AssetLayer(AssetLayer&&)            = delete;
    AssetLayer& operator=(AssetLayer&&) = delete;

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

AssetLoader::~AssetLoader() = default;

AssetLoader::AssetLoader(std::vector<fs::path> data_paths)
{
    m_asset_layers.reserve(data_paths.size());
    for (const fs::path& data_path : data_paths) {
        m_asset_layers.push_back(std::make_unique<AssetLayer>(data_path));
    }
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_config(std::string_view name)
{
    const std::array<std::string_view, 1> extensions{"XML"};
    return open_file(base_path() / "XML", name, extensions);
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_texture(std::string_view name)
{
    const std::array<std::string_view, 2> extensions{"DDS", "TGA"};
    return open_file(base_path() / "Art" / "Textures", name, extensions);
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_model(std::string_view name)
{
    const std::array<std::string_view, 1> extensions{"ALO"};
    return open_file(base_path() / "Art" / "Models", name, extensions);
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_shader(std::string_view name)
{
    const std::array<std::string_view, 1> extensions{"HLSL"};
    return open_file(base_path() / "Art" / "Shaders", name, extensions);
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_map(std::string_view name)
{
    const std::array<std::string_view, 1> extensions{"TED"};
    return open_file(base_path() / "Art" / "Maps", name, extensions);
}

std::unique_ptr<khepri::io::Stream>
AssetLoader::open_file(const fs::path& base_path, std::string_view name_,
                       gsl::span<const std::string_view> extensions)
{
    if (name_.empty()) {
        return {};
    }

    fs::path path = base_path / name_;
    for (auto& asset_layer : m_asset_layers) {
        if (auto file = asset_layer->open_file(path, extensions)) {
            LOG.info("Opened file \"{}\"", path.string());
            return file;
        }
    }

    LOG.error("unable to open file \"{}\"", path.string());
    return {};
}

AssetLoader::AssetLayer::AssetLayer(const std::filesystem::path& data_path)
    : m_data_path(data_path), m_megafs(data_path)
{
}

std::unique_ptr<khepri::io::Stream>
AssetLoader::AssetLayer::open_file(const std::filesystem::path&      path,
                                   gsl::span<const std::string_view> extensions)
{
    const auto& try_open_physical_file =
        [this](const std::filesystem::path& p) -> std::unique_ptr<khepri::io::Stream> {
        if (std::filesystem::exists(m_data_path / p)) {
            return std::make_unique<khepri::io::File>(m_data_path / p, khepri::io::OpenMode::read);
        }
        return nullptr;
    };

    const auto& try_open_mega_file =
        [this](const std::filesystem::path& p) -> std::unique_ptr<khepri::io::Stream> {
        return m_megafs.open_file(p);
    };

    const auto& try_with_extensions =
        [&](const auto& try_open_fn) -> std::unique_ptr<khepri::io::Stream> {
        if (auto stream = try_open_fn(path)) {
            return stream;
        }

        for (const auto& ext : extensions) {
            std::filesystem::path extended_path = path;
            extended_path.replace_extension(ext);
            if (auto stream = try_open_fn(extended_path)) {
                return stream;
            }
        }

        return nullptr;
    };

    if (auto stream = try_with_extensions(try_open_physical_file)) {
        return stream;
    }

    if (auto stream = try_with_extensions(try_open_mega_file)) {
        return stream;
    }

    return nullptr;
}

} // namespace openglyph
