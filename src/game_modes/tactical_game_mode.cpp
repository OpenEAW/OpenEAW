#include "dialogs/game_options_dialog.hpp"
#include "game_data.hpp"
#include "tactical_game_mode.hpp"

#include <openglyph/assets/io/map.hpp>
#include <openglyph/game/behaviors/marker_behavior.hpp>
#include <openglyph/game/behaviors/render_behavior.hpp>
#include <openglyph/game/scene.hpp>

#include <memory>

namespace openeaw {
namespace {
auto create_camera(const khepri::Size& render_size)
{
    const struct khepri::renderer::Camera::Properties properties = {
        khepri::renderer::Camera::Type::perspective,
        {100, 100, 150},
        {0, 0, 0},
        {0, 0, 1},
        khepri::to_radians(90.0F),
        0,
        static_cast<float>(render_size.width) / static_cast<float>(render_size.height),
        10.0F,
        100000.0F};
    return khepri::renderer::Camera{properties};
}

auto create_rts_camera(const GameData& game_data, khepri::renderer::Camera& camera)
{
    if (auto rts_camera = game_data.tactical_camera_store.create("Space_Mode", camera)) {
        return std::move(*rts_camera);
    }
    return khepri::game::RtsCameraController(camera, {0, 0});
}

std::unique_ptr<openglyph::Scene>
create_scene(std::string_view map_name, openglyph::AssetLoader& asset_loader,
             openglyph::AssetCache&                asset_cache,
             const openglyph::GameObjectTypeStore& game_object_types,
             khepri::game::RtsCameraController&    camera)
{
    if (auto stream = asset_loader.open_map(map_name)) {
        const auto             map = openglyph::io::read_map(*stream);
        openglyph::Environment environment{};
        if (!map.environments.empty()) {
            assert(map.active_environment < map.environments.size());
            environment = map.environments[map.active_environment];
        }

        auto scene =
            std::make_unique<openglyph::Scene>(asset_cache, game_object_types, environment);

        for (const auto& obj : map.objects) {
            if (const auto* type = game_object_types.get(obj.type_crc)) {
                auto object = std::make_shared<khepri::scene::SceneObject>();

                // Store a (dumb, non-owning) reference to the GameObjectType
                object->user_data(type);

                if (const auto* render_model =
                        asset_cache.get_render_model(type->space_model_name)) {
                    auto& behavior =
                        object->create_behavior<openglyph::RenderBehavior>(*render_model);
                    behavior.scale(type->scale_factor);
                    if (type->is_in_background) {
                        behavior.render_layer(openglyph::RenderBehavior::RenderLayer::background);
                    }
                }

                if (type->is_marker) {
                    object->create_behavior<openglyph::MarkerBehavior>();
                }

                object->rotation(obj.facing);
                object->position(obj.position);
                scene->add_object(std::move(object));
            }
        }

        // Find the first "player 0" marker to place the camera at.
        for (const auto& object : scene->objects<openglyph::MarkerBehavior>()) {
            if (const auto** type_ptr = object->user_data<const openglyph::GameObjectType*>()) {
                if (khepri::case_insensitive_equals((*type_ptr)->name,
                                                    "Player_0_Spawn_Point_Marker")) {
                    camera.target({object->position().x, object->position().y});
                    break;
                }
            }
        }

        return scene;
    }
    return {};
}

auto& get_render_pipeline(openglyph::AssetCache& asset_cache, std::string_view name)
{
    auto render_pipeline = asset_cache.get_render_pipeline("Default");
    if (!render_pipeline) {
        // We can't render without a pipeline, so this is a fatal error
        throw std::runtime_error("Unable to load default render pipeline");
    }
    return *render_pipeline;
}

/**
 * An invisible widget that captures every input and forwards it to a handler specified at
 * construction. This is used to capture all events that don't hit a UI element and e.g. turn it
 * into game controls (camera rotation or unit selection).
 */
class EventCaptureWidget : public khepri::ui::Widget
{
public:
    EventCaptureWidget(const khepri::ui::Layout&              layout,
                       std::function<void(khepri::ui::Event)> event_handler)
        : khepri::ui::Widget(layout), m_event_handler(std::move(event_handler))
    {
        assert(m_event_handler);
    }

protected:
    bool can_focus() override
    {
        // So there's always something to focus, so we receive key events
        return true;
    }

private:
    void on_event(const khepri::ui::Event& event) override
    {
        // Make sure we track the mouse if it moves out of bounds, if a button is pressed.
        // This is necessary e.g. so that drag operations continue even if the mouse exits the game
        // window, which is the expected behavior.
        if (std::holds_alternative<khepri::ui::MousePressEvent>(event)) {
            set_capture();
        } else if (std::holds_alternative<khepri::ui::MouseReleaseEvent>(event)) {
            release_capture();
        }

        m_event_handler(event);
    }

    std::function<void(khepri::ui::Event)> m_event_handler;
};

} // namespace

TacticalGameMode::TacticalGameMode(const GameContext& context, const GameData& game_data)
    : GameMode(context, game_data.string_database)
    , m_camera(create_camera(context.renderer.render_size()))
    , m_render_size_connection(
          context.renderer.add_render_size_listener([&](const khepri::Size& size) {
              m_camera.aspect(static_cast<float>(size.width) / size.height);
          }))
    , m_rts_camera(create_rts_camera(game_data, m_camera))
    , m_tactical_mode_input_handler(m_rts_camera, context.window)
    , m_scene(create_scene("_MP_SPACE_ALDERAAN", context.asset_loader, context.asset_cache,
                           game_data.game_object_types, m_rts_camera))
    , m_scene_renderer(context.renderer, get_render_pipeline(context.asset_cache, "Default"))
{
    // First add the capture widget to the canvas. This will catch all mouse and key events that
    // aren't used by dialogs, etc.
    ui().canvas().add(std::make_shared<EventCaptureWidget>(
        khepri::ui::Layout::fill(), [this](const auto& event) { on_event(event); }));
}

bool TacticalGameMode::on_event(const khepri::ui::Event& event)
{
    if (const auto* key = std::get_if<khepri::ui::KeyPressEvent>(&event)) {
        if (key->code == khepri::ui::KeyCode::escape) {
            // Escape was pressed, bring up the options dialog
            ui().activate_dialog<GameOptionsDialog>([this] { close(); });
            return true;
        }
    }

    return m_tactical_mode_input_handler.handle_event(event);
}

void TacticalGameMode::update(double delta_time)
{
    GameMode::update(delta_time);

    m_rts_camera.update(delta_time);
}

void TacticalGameMode::render()
{
    if (m_scene) {
        m_scene_renderer.render_scene(*m_scene, m_camera);
    }
    context().renderer.clear(khepri::renderer::Renderer::clear_depth);

    // Now render the UI on top of the scene
    GameMode::render();
}

} // namespace openeaw
