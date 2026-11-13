#pragma once

#include "game_data.hpp"
#include "game_modes/game_mode.hpp"

#include <khepri/renderer/renderer.hpp>
#include <khepri/ui/events.hpp>
#include <openglyph/assets/asset_cache.hpp>
#include <openglyph/assets/asset_loader.hpp>

#include <memory>

namespace openeaw {

class GameModeStack : public khepri::ui::InputEventHandler
{
public:
    GameModeStack(const GameContext& context, const GameData& game_data)
        : m_context(context), m_game_data(game_data)
    {
    }

    [[nodiscard]] bool has_active_game_mode() const noexcept
    {
        return !m_game_modes.empty();
    }

    void render()
    {
        if (!m_game_modes.empty()) {
            m_game_modes.back()->render();
        }
    }

    void update(double delta_time)
    {
        if (!m_game_modes.empty()) {
            m_game_modes.back()->update(delta_time);
        }
    }

    template <typename GameModeType, typename... Args>
    void activate_game_mode(Args&&... args)
    {
        auto game_mode =
            std::make_unique<GameModeType>(m_context, m_game_data, std::forward<Args>(args)...);
        if (!m_game_modes.empty()) {
            m_game_modes.back()->on_deactivated();
        }
        game_mode->add_close_listener([this, game_mode_ptr = game_mode.get()] {
            assert(!m_game_modes.empty());
            assert(m_game_modes.back().get() == game_mode_ptr);
            game_mode_ptr->on_deactivated();
            m_game_modes.pop_back();
            if (!m_game_modes.empty()) {
                m_game_modes.back()->on_activated();
            }
        });
        m_game_modes.push_back(std::move(game_mode));
        m_game_modes.back()->on_activated();
    }

    bool handle_event(const khepri::ui::InputEvent& event) override
    {
        // Dispatch to the active game mode (if any)
        if (!m_game_modes.empty()) {
            return m_game_modes.back()->handle_event(event);
        }
        return false;
    }

private:
    const GameContext& m_context;
    const GameData&    m_game_data;

    std::vector<std::unique_ptr<openeaw::GameMode>> m_game_modes;
};

} // namespace openeaw