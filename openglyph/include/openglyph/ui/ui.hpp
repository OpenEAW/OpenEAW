#pragma once

#include "context.hpp"
#include "dialog_store.hpp"
#include "dialogs.hpp"
#include "style_store.hpp"

#include <khepri/adapters/ui_renderer.hpp>
#include <khepri/exceptions.hpp>
#include <khepri/renderer/renderer.hpp>
#include <khepri/ui/canvas.hpp>
#include <khepri/ui/events.hpp>

#include <openglyph/assets/asset_cache.hpp>
#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/assets/mega_texture.hpp>

#include <memory>
#include <type_traits>

namespace openglyph::ui {
namespace detail {

template <typename, typename = std::void_t<>>
struct HasBeforeDialogCreation : std::false_type
{};

template <typename T>
struct HasBeforeDialogCreation<T, std::void_t<decltype(T::before_dialog_creation)>> : std::true_type
{};

} // namespace detail

class DialogCreator;

class UserInterface final : public khepri::ui::InputEventHandler
{
    class ModalContainer : public khepri::ui::Widget
    {
        using khepri::ui::Widget::Widget;
    };

public:
    UserInterface(openglyph::AssetLoader& asset_loader, openglyph::AssetCache& asset_cache,
                  const openglyph::StringDatabase& string_database,
                  khepri::renderer::Renderer&      renderer);
    ~UserInterface();

    UserInterface(const UserInterface&)            = delete;
    UserInterface(UserInterface&&)                 = delete;
    UserInterface& operator=(const UserInterface&) = delete;
    UserInterface& operator=(UserInterface&&)      = delete;

    bool handle_event(const khepri::ui::InputEvent& event) override;

    /**
     * Creates and activates the modal dialog and its controller. The controller is identified by
     * DialogType and the dialog itself is created from the dialog resource identified by
     * DialogType::RESOURCE_NAME.
     *
     * The dialog is placed on top of all other elements in the UI. The dialog is modal: all UI
     * elements below the dialog do not receive input events anymore.
     *
     * The arguments in this call are passed to the constructor of the dialog controller.
     *
     * If the dialog resource cannot be found, this function does nothing.
     */
    template <typename DialogType, typename... Args>
    void activate_dialog(Args&&... args)
    {
        if (auto* dialog_def = m_dialog_store.find_dialog(DialogType::RESOURCE_NAME)) {
            if constexpr (detail::HasBeforeDialogCreation<DialogType>::value) {
                // Allow the dialog type to optionally perform some setup before the dialog is
                // created and added to the canvas
                DialogType::before_dialog_creation(m_context);
            }

            auto dialog_widget = create_dialog_widget(*dialog_def, DialogType::DIALOG_POSITION);
            auto dialog_obj    = std::make_unique<Dialog>(*this, dialog_widget);
            auto controller =
                std::make_unique<DialogType>(*dialog_obj, std::forward<Args>(args)...);

            if (!m_active_dialogs.empty()) {
                m_active_dialogs.back().controller->on_deactivated();
            }

            auto modal_container = std::make_shared<khepri::ui::Frame>(khepri::ui::Layout::fill());
            modal_container->add(dialog_widget);

            dialog_obj->add_close_listener(
                [this, modal_container, dialog_ptr = dialog_obj.get()]() {
                    assert(!m_active_dialogs.empty());
                    assert(m_active_dialogs.back().dialog.get() == dialog_ptr);
                    m_active_dialogs.back().controller->on_deactivated();
                    m_active_dialogs.pop_back();
                    if (!m_active_dialogs.empty()) {
                        m_active_dialogs.back().controller->on_activated();
                    }
                    m_canvas.remove(modal_container);
                });

            m_active_dialogs.push_back({std::move(dialog_obj), std::move(controller)});
            m_active_dialogs.back().controller->on_activated();

            m_canvas.add(modal_container);
        }
    }

    [[nodiscard]] bool has_active_dialogs() const noexcept
    {
        return !m_active_dialogs.empty();
    }

    void render();

    [[nodiscard]] khepri::ui::Canvas& canvas() noexcept
    {
        return m_canvas;
    }

    [[nodiscard]] khepri::UIRenderer& renderer() noexcept
    {
        return m_ui_renderer;
    }

private:
    khepri::ui::InputEvent scale_event(const khepri::ui::InputEvent& event) const noexcept;
    void                   scale_position(khepri::ui::MousePosition& position) const noexcept;

    std::shared_ptr<khepri::ui::Widget> create_dialog_widget(const DialogDefinition& def,
                                                             DialogPosition          position);

    openglyph::AssetCache&    m_asset_cache;
    khepri::UIRenderer        m_ui_renderer;
    khepri::ui::Canvas        m_canvas;
    openglyph::ui::StyleStore m_style_store;
    Context                   m_context;

    openglyph::ui::DialogStore     m_dialog_store;
    std::unique_ptr<DialogCreator> m_dialog_creator;

    struct DialogState
    {
        std::unique_ptr<Dialog>           dialog;
        std::unique_ptr<DialogController> controller;
    };

    std::vector<DialogState> m_active_dialogs;
};

} // namespace openglyph::ui