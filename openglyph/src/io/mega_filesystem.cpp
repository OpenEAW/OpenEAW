#include <khepri/log/log.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/io/mega_file.hpp>
#include <openglyph/io/mega_filesystem.hpp>
#include <openglyph/parser/xml_parser.hpp>

namespace openglyph::io {
const khepri::log::Logger LOG("megafs");

MegaFileSystem::MegaFileSystem(std::vector<std::filesystem::path> data_paths)
    : m_data_paths(std::move(data_paths))
{
    for (const auto& data_path : m_data_paths) {
        const std::filesystem::path index_file = data_path / "Data/megafiles.xml";
        if (std::filesystem::exists(index_file)) {
            khepri::io::File file(index_file, khepri::io::OpenMode::read);
            parse_index_file(file);
        }
    }
}

std::unique_ptr<khepri::io::Stream> MegaFileSystem::open_file(std::filesystem::path& path)
{
    for (auto& mega_file : m_mega_files) {
        std::unique_ptr<khepri::io::Stream> stream = mega_file->open_file(path);
        if (stream) {
            return stream;
        }
    }
    return nullptr;
}

void MegaFileSystem::parse_index_file(khepri::io::Stream& stream)
{
    XmlParser parser(stream);
    if (const auto& root = parser.root()) {
        for (const auto& node : root->nodes()) {
            bool file_found = false;

            std::string_view sub_path = node.value();

            for (const auto& data_path : m_data_paths) {
                // TODO update to support DVD version
                //  the megafiles.xml sub_path is encapsulated by spaces, trim the sub_path

                // the filenames are all lowercase for steam.
                std::filesystem::path full_path = data_path / khepri::trim(sub_path);

                if (!std::filesystem::exists(full_path)) {
                    continue;
                }

                std::string filename = khepri::lowercase(full_path.filename().string());

                full_path.replace_filename(filename);
                m_mega_files.emplace_back(std::make_unique<MegaFile>(full_path));

                file_found = true;
                break;
            }

            if (!file_found) {
                LOG.error("unable to open file \"{}\"", sub_path);
            }
        }
    }
}
} // namespace openglyph::io