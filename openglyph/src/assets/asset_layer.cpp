#include <openglyph/assets/asset_layer.hpp>
namespace openglyph {

AssetLayer::AssetLayer(const std::filesystem::path& data_path)
    : m_data_path(data_path), m_megafs(std::make_unique<io::MegaFileSystem>(data_path))
{
}

std::unique_ptr<khepri::io::Stream>
AssetLayer::open_file(const std::filesystem::path&      path,
                      gsl::span<const std::string_view> extensions)
{
    const auto& try_open_physical_file =
        [this](const std::filesystem::path& data_path,
               const std::filesystem::path& path) -> std::unique_ptr<khepri::io::Stream> {
        if (std::filesystem::exists(data_path / path)) {
            // Physical file exists
            return std::make_unique<khepri::io::File>(data_path / path, khepri::io::OpenMode::read);
        }

        return nullptr;
    };

    const auto& try_open_mega_file =
        [this](const std::filesystem::path& path) -> std::unique_ptr<khepri::io::Stream> {
        if (m_megafs) {
            return m_megafs->open_file(path);
        }
        return nullptr;
    };

    // Try opening the physical file.
    if (auto stream = try_open_physical_file(m_data_path, path)) {
        return stream;
    }

    // Try with various extensions
    for (const auto& extension : extensions) {
        std::filesystem::path extended_path = path;
        extended_path.replace_extension(extension);

        if (auto stream = try_open_physical_file(m_data_path, extended_path)) {
            return stream;
        }
    }

    // Try opening the mega file.
    if (auto stream = try_open_mega_file(path)) {
        return stream;
    }

    // Try with various extensions
    for (const auto& extension : extensions) {
        std::filesystem::path extended_path = path;
        extended_path.replace_extension(extension);

        if (auto stream = try_open_mega_file(extended_path)) {
            return stream;
        }
    }

    return nullptr;
}
} // namespace openglyph
