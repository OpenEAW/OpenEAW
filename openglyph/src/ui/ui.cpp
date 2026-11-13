#include "dialog_creator.hpp"

#include <khepri/log/log.hpp>
#include <khepri/renderer/io/texture.hpp>
#include <khepri/ui/button.hpp>
#include <khepri/ui/label.hpp>

#include <openglyph/assets/io/mega_texture.hpp>
#include <openglyph/ui/ui.hpp>

#include <cmath>

namespace openglyph::ui {
namespace {
constexpr khepri::log::Logger LOG("ui");

static constexpr khepri::Size CANVAS_SIZE{1024u, 768u};
} // namespace

UserInterface::UserInterface(openglyph::AssetLoader&          asset_loader,
                             openglyph::AssetCache&           asset_cache,
                             const openglyph::StringDatabase& string_database,
                             khepri::renderer::Renderer&      renderer)
    : m_asset_cache(asset_cache)
    , m_ui_renderer(renderer, asset_cache.get_render_pipeline("Default"))
    , m_canvas(CANVAS_SIZE, m_ui_renderer)
    , m_style_store(asset_loader, "GUIDialogs.xml")
    , m_context(m_canvas, m_ui_renderer, asset_loader, asset_cache, string_database, m_style_store)
    , m_dialog_store(asset_loader, "GUIDialog\\GUIDialogs.rc", "GUIDialog\\Resource.h")
{
    m_dialog_creator = std::make_unique<DialogCreator>(m_context);
}

UserInterface::~UserInterface() = default;

bool UserInterface::handle_event(const khepri::ui::InputEvent& event)
{
    const auto scaled_event = scale_event(event);
    return m_canvas.handle_event(scaled_event);
}

khepri::ui::InputEvent
UserInterface::scale_event(const khepri::ui::InputEvent& event) const noexcept
{
    // Scale the mouse position (if any) from window space to canvas space
    auto scaled_event = event;
    if (auto* mme = std::get_if<khepri::ui::MouseMoveEvent>(&scaled_event)) {
        scale_position(mme->position);
    } else if (auto* mbe = std::get_if<khepri::ui::MouseButtonEvent>(&scaled_event)) {
        scale_position(mbe->position);
    } else if (auto* mse = std::get_if<khepri::ui::MouseScrollEvent>(&scaled_event)) {
        scale_position(mse->position);
    }
    return scaled_event;
}

void UserInterface::scale_position(khepri::ui::MousePosition& position) const noexcept
{
    // Scale the mouse position from window space to canvas space
    const auto& render_size = m_ui_renderer.render_size();
    position.x =
        std::lround(position.x * static_cast<double>(CANVAS_SIZE.width) / render_size.width);
    position.y =
        std::lround(position.y * static_cast<double>(CANVAS_SIZE.height) / render_size.height);
}

void UserInterface::render()
{
    m_canvas.render();
}

std::shared_ptr<khepri::ui::Widget> UserInterface::create_dialog_widget(const DialogDefinition& def,
                                                                        DialogPosition position)
{
    return m_dialog_creator->create_dialog_widget(def, position);
}

} // namespace openglyph::ui