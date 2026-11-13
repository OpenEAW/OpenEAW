#pragma once

#include "dialogs.hpp"

#include <khepri/utility/string.hpp>

#include <openglyph/assets/asset_loader.hpp>

#include <map>
#include <string_view>

namespace openglyph::ui {

/**
 * \brief A centralized store for UI dialogs.
 */
class DialogStore final
{
public:
    /**
     * Constructs the Dialog Store and loads all dialog definitions.
     *
     * The UI dialogs are defined in two files: an .RC resource file and a .H header file. These
     * form the typical Windows resource pair for dialogs.
     */
    DialogStore(AssetLoader& asset_loader, std::string_view rc_filename,
                std::string_view h_filename);

    const DialogDefinition* find_dialog(std::string_view name) const noexcept;

private:
    std::map<std::string, DialogDefinition, khepri::CaseInsensitiveLess> m_dialogs;
};

} // namespace openglyph::ui