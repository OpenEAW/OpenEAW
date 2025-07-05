#include <khepri/log/log.hpp>
#include <khepri/utility/string.hpp>

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

std::unique_ptr<SubFile> MegaFileSystem::open_file(std::filesystem::path& path)
{
    for (auto& mega_file : m_mega_files) {
        try {
            return mega_file->open_file(path);
        } catch (khepri::io::Error&) {
        }
    }
    throw khepri::io::Error("subfile not found");
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

                    std::string filename = khepri::lowercase(full_path.filename().string());

                    full_path.replace_filename(filename);
                    m_mega_files.emplace_back(std::make_unique<MegaFile>(full_path));
                } catch (khepri::io::Error&) {
                }
            }
        }
    }
}

MegaFile::MegaFile(const std::filesystem::path& mega_file_path)
    : m_megaFile(mega_file_path, khepri::io::OpenMode::read)
{
    extract_metadata(filenames, fileinfo);

    // calculate hashes for this session
    for (const SubFileInfo& info : fileinfo) {
        std::size_t hash = std::hash<std::string>{}(filenames[info.file_name_index]);
        file_hashes.push_back(hash);
    }
}

std::unique_ptr<SubFile> MegaFile::open_file(std::filesystem::path& path)
{
    std::size_t hash = std::hash<std::string>{}(khepri::uppercase(path.string()));
    auto        it   = std::find(file_hashes.begin(), file_hashes.end(), hash);

    if (it != file_hashes.end()) {
        std::size_t index = static_cast<std::size_t>(std::distance(file_hashes.begin(), it));
        return std::make_unique<SubFile>(fileinfo[index], &m_megaFile);
    }
    throw khepri::io::Error("subfile not found");
}
void MegaFile::extract_metadata(std::vector<std::string>&                filenames,
                                std::vector<openglyph::io::SubFileInfo>& fileinfo)
{
    file_name_count = m_megaFile.read_uint32();
    file_info_count = m_megaFile.read_uint32();

    for (size_t i = 0; i < file_name_count; i++) {
        filenames.emplace_back(m_megaFile.read_string());
    }

    for (size_t i = 0; i < file_name_count; i++) {
        SubFileInfo info = {m_megaFile.read_uint32(), m_megaFile.read_uint32(),
                            m_megaFile.read_uint32(), m_megaFile.read_uint32(),
                            m_megaFile.read_uint32()};

        fileinfo.emplace_back(info);
    }
}

SubFile::SubFile(const SubFileInfo info, khepri::io::File* p_file) : info(info), p_megaFile(p_file)
{
}

size_t SubFile::read(void* buffer, size_t count)
{
    try {
        // Another file could have read from the current file, so update the read head.
        p_megaFile->seek(info.file_offset + local_offset, khepri::io::SeekOrigin::begin);

        // Count could be larger then the file, prevent reading past the edge of the sub file
        if (local_offset + count > info.file_size) {
            count = info.file_size - local_offset;
        }

        return p_megaFile->read(buffer, count);
    } catch (Error) {
    }
    return 0;
}

size_t SubFile::write(const void* buffer, size_t count) 
{
    throw khepri::io::Error("writing is not supported");
}

long long SubFile::seek(long long offset, khepri::io::SeekOrigin origin)
{
    switch (origin)
    {
    case khepri::io::SeekOrigin::begin:
        p_megaFile->seek(info.file_offset + offset, khepri::io::SeekOrigin::begin);
        /* code */
        break;
    case khepri::io::SeekOrigin::current:
        p_megaFile->seek(info.file_offset + local_offset + offset, khepri::io::SeekOrigin::begin);

        /* code */
        break;
    case khepri::io::SeekOrigin::end:
        p_megaFile->seek(info.file_offset + info.file_size - offset, khepri::io::SeekOrigin::begin);
        break;
    
    default:
        break;
    }
    return 0;
}

} // namespace openglyph::io
