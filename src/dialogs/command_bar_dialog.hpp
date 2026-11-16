#pragma once

#include "game_mode_stack.hpp"

#include <openglyph/ui/context.hpp>

#include <vector>
namespace openeaw {

class CommandBar
{
public:
    explicit CommandBar(openglyph::AssetLoader& asset_loader);

private:
    [[nodiscard]] std::vector<std::string_view>
    load_component_files(openglyph::AssetLoader& asset_loader);
};

} // namespace openeaw
