#pragma once

#include "game_data.hpp"
#include "game_mode.hpp"

#include <openglyph/game/scene_renderer.hpp>
#include <openglyph/ui/input.hpp>

namespace openeaw {

/**
 * This class is the "game mode" for tactical battles.
 */
class TacticalGameMode final : public GameMode
{
public:
    TacticalGameMode(const GameContext& context, const GameData& game_data);

    void render() override;
    void update(double delta_time) override;

private:
    bool on_event(const khepri::ui::Event& event);

    khepri::renderer::Camera                m_camera;
    khepri::ScopedConnection                m_render_size_connection;
    khepri::game::RtsCameraController       m_rts_camera;
    openglyph::ui::TacticalModeInputHandler m_tactical_mode_input_handler;
    std::unique_ptr<openglyph::Scene>       m_scene;
    openglyph::SceneRenderer                m_scene_renderer;
};

} // namespace openeaw
