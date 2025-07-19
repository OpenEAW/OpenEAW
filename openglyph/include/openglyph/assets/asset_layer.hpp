#include <openglyph/io/mega_filesystem.hpp>
namespace openglyph {

/**
 * @brief Provides access to asset files from a single data source (physical or MegaFile).
 *
 * Each AssetLayer represents one data path.
 *
 * The class abstracts the file-loading logic by attempting to open files in the following order:
 * 1. From the physical file system rooted at `data_path`.
 * 2. From a MegaFile archive using `MegaFileSystem`.
 *
 */
class AssetLayer
{
public:
    explicit AssetLayer(const std::filesystem::path& data_path);

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
     * @param extensions A list of alternative file extensions to try if the base file is not found
     * or an extension is not provided in the path.
     * @return A unique pointer to a Stream representing the opened file, or nullptr if no file
     * could be opened.
     */
    std::unique_ptr<khepri::io::Stream> open_file(const std::filesystem::path&      path,
                                                  gsl::span<const std::string_view> extensions);

private:
    std::filesystem::path               m_data_path{};
    std::unique_ptr<io::MegaFileSystem> m_megafs = nullptr;
};
} // namespace openglyph