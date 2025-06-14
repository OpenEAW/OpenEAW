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
khepri::log::Logger LOG("assets");
const fs::path BASE_PATH = "Data";
}

AssetLoader::AssetLoader(std::vector<fs::path> data_paths) : m_data_paths(std::move(data_paths)) {}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_config(std::string_view name)
{
    const std::array<std::string_view, 1> extensions{"xml"};
    return open_file(BASE_PATH / "XML", name, extensions);
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_texture(std::string_view name)
{
    const std::array<std::string_view, 2> extensions{"dds", "tga"};
    return open_file(BASE_PATH / "Art" / "Textures", name, extensions);
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_model(std::string_view name)
{
    const std::array<std::string_view, 1> extensions{"alo"};
    return open_file(BASE_PATH / "Art" / "Models", name, extensions);
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_shader(std::string_view name)
{
    const std::array<std::string_view, 1> extensions{"hlsl"};
    return open_file(BASE_PATH / "Art" / "Shaders", name, extensions);
}

std::unique_ptr<khepri::io::Stream> AssetLoader::open_map(std::string_view name)
{
    const std::array<std::string_view, 1> extensions{""};
    return open_file(BASE_PATH / "Art" / "Maps", name, extensions);
}

std::unique_ptr<khepri::io::Stream>
AssetLoader::open_file(const fs::path& base_path, std::string_view name_,
                       gsl::span<const std::string_view> extensions)
{
    if (name_.empty()) {
        return {};
    }
    
    //on windows paths are case sensitive. (so we cannot use uppercase)
    auto path = base_path / name_;

    const auto& try_open_file = [this](const fs::path& path) -> std::unique_ptr<khepri::io::File> {
        for (const auto& data_path : m_data_paths) {
            try {
                auto file = std::make_unique<khepri::io::File>(data_path / path,
                                                               khepri::io::OpenMode::read);
                return file;
            } catch (khepri::io::Error&) {
            }
        }
        return {};
    };

    // Try as-is
    if (auto file = try_open_file(path)) {
        LOG.info("Opened file \"{}\"", path.string());
        return file;
    }

    // Try with the various extensions
    for (const auto& extension : extensions) {
        path.replace_extension(extension);
        if (auto file = try_open_file(path)) {
            LOG.info("Opened file \"{}\"", path.string());
            return file;
        }
    }

    LOG.error("unable to open file \"{}\"", path.string());
    return {};
}

} // namespace openglyph
