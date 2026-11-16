#include "command_bar_dialog.hpp"

#include <openglyph/parser/parsers.hpp>

namespace openeaw {
constexpr khepri::log::Logger LOG("font");

std::vector<std::string_view> CommandBar::load_component_files(openglyph::AssetLoader& asset_loader)
{
    std::vector<std::string_view> component_files;
    if (auto stream = asset_loader.open_config("CommandBarComponentFiles")) {
        openglyph::XmlParser parser(*stream);
        if (const auto& root = parser.root()) {
            for (const auto& node : root->nodes()) {
                try {
                    if (node.name() != "File") {
                        LOG.error("Expected File node");
                        continue;
                    }
                    component_files.push_back(node.value());
                } catch (openglyph::ParseError& e) {
                    LOG.error("Error reading command bar component files cameras: {}", e.what());
                }
            }
        }
    }
    return component_files;
}

CommandBar::CommandBar(openglyph::AssetLoader& asset_loader)
{
    for (const auto& component_file : load_component_files(asset_loader)) {
        // TODO load the files.
    }
}
} // namespace openeaw