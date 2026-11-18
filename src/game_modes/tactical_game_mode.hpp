#pragma once

#include "game_mode.hpp"

#include <openglyph/game/player_list.hpp>
#include <openglyph/game/scene_renderer.hpp>
#include <openglyph/ui/input.hpp>

namespace openeaw {

/**
 * This class is the "game mode" for tactical battles.
 */
class TacticalGameMode final : public GameMode
{
public:
    struct GameOptions
    {};

    explicit TacticalGameMode(const GameContext& context, openglyph::PlayerList& players,
                              const GameOptions& options);

    void render() override;
    void update(double delta_time) override;

private:
    void set_initial_camera();
    bool on_event(const khepri::ui::Event& event);

    openglyph::PlayerList&                  m_players;
    const GameOptions                       m_options;
    khepri::renderer::Camera                m_camera;
    khepri::ScopedConnection                m_render_size_connection;
    khepri::game::RtsCameraController       m_rts_camera;
    openglyph::ui::TacticalModeInputHandler m_tactical_mode_input_handler;
    std::unique_ptr<openglyph::Scene>       m_scene;
    openglyph::SceneRenderer                m_scene_renderer;
};

} // namespace openeaw
