#include "game_mode.hpp"

#include <chrono>

namespace openeaw {
namespace {
// Time, in seconds, between each 'game logic' update step.
constexpr auto UPDATE_STEP_TIME = 1.0 / 60;
} // namespace

GameMode::GameMode(const GameContext& context, const openglyph::StringDatabase& string_database)
    : m_context(context)
    , m_ui(context.asset_loader, context.asset_cache, string_database, context.renderer)
{
}

bool GameMode::handle_event(const khepri::ui::InputEvent& event)
{
    return m_ui.handle_event(event);
}

void GameMode::render()
{
    m_ui.render();
}

void GameMode::update(double delta_time) {}

void GameMode::close()
{
    for (const auto& listener : m_close_listeners) {
        listener();
    }
}

} // namespace openeaw