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
            auto file = std::make_unique<khepri::io::File>(data_path / "megafiles.xml",
                                                           khepri::io::OpenMode::read);
            LOG.info("loaded file");
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
            LOG.info(node.value());
        }
    }
}
} // namespace openglyph::io