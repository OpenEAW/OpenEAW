#include <khepri/log/log.hpp>
#include <openglyph/io/mega_filesystem.hpp>
#include <openglyph/parser/xml_parser.hpp>
namespace openglyph::io {
khepri::log::Logger LOG("megafs");

MegaFileSystem::MegaFileSystem(std::vector<std::filesystem::path> data_paths)
    : m_data_paths(std::move(data_paths))
{
    for (const auto& data_path : m_data_paths) {
        try {
            auto file = std::make_unique<khepri::io::File>(data_path / "Data/megafiles.xml",
                                                           khepri::io::OpenMode::read);
            parse_index_file(*file);
        } catch (khepri::io::Error&) {
        }
    }
}
void MegaFileSystem::parse_index_file(khepri::io::Stream& stream)
{
    XmlParser parser(stream);
    if (const auto& root = parser.root()) {
        for (const auto& node : root->nodes()) {
            for (const auto& data_path : m_data_paths) {
                try {
                    std::string_view sub_path = node.value();

                    // the megafiles.xml is encapsulated by spaces
                    sub_path.remove_prefix(1);
                    sub_path.remove_suffix(1);

                    // the filenames are all lowercase.
                    std::filesystem::path full_path = data_path / sub_path;

                    std::string filename = full_path.filename().string();

                    std::transform(filename.begin(), filename.end(), filename.begin(),
                                   [](unsigned char c) { return std::tolower(c); });

                    full_path.replace_filename(filename);
                    m_mega_files.emplace_back(std::make_shared<MegaFile>(full_path));
                } catch (khepri::io::Error&) {
                }
            }
        }
    }
}

MegaFile::MegaFile(const std::filesystem::path& mega_file_path)
    : m_megaFile(mega_file_path, khepri::io::OpenMode::read)
{
    file_name_count = m_megaFile.read_uint32();
    file_info_count = m_megaFile.read_uint32();

    for (size_t i = 0; i < file_name_count; i++) {
        filenames.emplace_back(m_megaFile.read_string());
    }

    for (size_t i = 0; i < file_name_count; i++) {
        read_file_info();
    }
}

void MegaFile::read_file_info()
{
    SubFileInfo info = {m_megaFile.read_uint32(), m_megaFile.read_uint32(),
                        m_megaFile.read_uint32(), m_megaFile.read_uint32(),
                        m_megaFile.read_uint32()};

    fileinfo.emplace_back(info);
}
} // namespace openglyph::io
