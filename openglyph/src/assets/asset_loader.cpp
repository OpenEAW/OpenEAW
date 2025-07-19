#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/log/log.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/assets/asset_loader.hpp>

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

AssetLoader::AssetLoader(std::vector<fs::path> data_paths)
{
    m_asset_layers.reserve(data_paths.size());
    for (const fs::path& data_path : data_paths) {
        m_asset_layers.emplace_back(AssetLayer(data_path));
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
        if (auto file = asset_layer.open_file(path, extensions)) {
            LOG.info("Opened file \"{}\"", path.string());
            return file;
        }
    }

    LOG.error("unable to open file \"{}\"", path.string());
    return {};
}

} // namespace openglyph
