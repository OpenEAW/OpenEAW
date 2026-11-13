#include <openglyph/ui/dialogs.hpp>
#include <openglyph/ui/ui.hpp>

namespace openglyph::ui {

void Dialog::close()
{
    for (auto& listener : m_close_listeners) {
        listener();
    }
}

} // namespace openglyph::ui