#include <khepri/log/log.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/io/mega_file.hpp>
#include <openglyph/io/mega_filesystem.hpp>
#include <openglyph/parser/xml_parser.hpp>

namespace openglyph::io {
const khepri::log::Logger LOG("megafs");

MegaFileSystem::MegaFileSystem(const std::filesystem::path& data_path) : m_data_path(data_path)
{
    // The paths in megafiles.xml are lowercase for steam.
    const std::filesystem::path index_file = m_data_path / "Data" / "megafiles.xml";
    if (std::filesystem::exists(index_file)) {
        khepri::io::File file(index_file, khepri::io::OpenMode::read);
        parse_index_file(data_path, file);
    }
}

std::unique_ptr<khepri::io::Stream> MegaFileSystem::open_file(const std::filesystem::path& path)
{
    for (const auto& mega_file : m_mega_files) {
        if (std::unique_ptr<khepri::io::Stream> stream = mega_file->open_file(path)) {
            return stream;
        }
    }

    return nullptr;
}

void MegaFileSystem::parse_index_file(const std::filesystem::path& data_path,
                                      khepri::io::Stream&          stream)
{
    XmlParser parser(stream);
    if (const auto& root = parser.root()) {
        for (const auto& node : root->nodes()) {
            std::string_view sub_path = node.value();

            //  the megafiles.xml sub_path is encapsulated by spaces, trim the sub_path

            // the filenames are all lowercase for steam.
            std::filesystem::path full_path = data_path / khepri::trim(sub_path);

            std::string filename = khepri::lowercase(full_path.filename().string());
            full_path.replace_filename(filename);

            if (!std::filesystem::exists(full_path)) {
                LOG.error("Cannot open megafile \"{}\"", sub_path);
            } else {
                m_mega_files.push_back(std::make_unique<MegaFile>(full_path));
            }
        }
    }
}
} // namespace openglyph::io