#pragma once

#include "game_context.hpp"

#include <openglyph/assets/string_database.hpp>
#include <openglyph/ui/ui.hpp>

#include <vector>

namespace openeaw {

/**
 * \brief Abstract base class for game modes.
 *
 * A game mode represents a particular mode of operation of the game, such as the main menu,
 * tactical game, galactic game, multiplayer game, etc. A game mode is 'modal' and, as such, has its
 * own main loop.
 */
class GameMode
{
public:
    /// Listener that's called when the game mode is closed.
    using CloseListener = std::function<void()>;

    GameMode(const GameContext& context, const openglyph::StringDatabase& string_database);

    virtual ~GameMode() = default;

    /**
     * Called when the game mode is activated, or when it is re-activated after being deactivated
     * (i.e. when another game mode has been closed).
     */
    virtual void on_activated() {}

    /**
     * Called when the game mode is deactivated (i.e. when another game mode is activated on top of
     * it, or when it is closed).
     */
    virtual void on_deactivated() {}

    /**
     * Called to handle input events. Should return true if the event was handled.
     */
    virtual bool handle_event(const khepri::ui::InputEvent& event);

    /**
     * Renders the current frame.
     * Should be implemented by derived classes to render the game scene.
     */
    virtual void render();

    /**
     * Called to update the game state. Should be implemented by derived classes to update the game.
     */
    virtual void update(double delta_time);

    void add_close_listener(CloseListener listener)
    {
        m_close_listeners.push_back(listener);
    }

protected:
    const GameContext& context() noexcept
    {
        return m_context;
    }

    openglyph::ui::UserInterface& ui() noexcept
    {
        return m_ui;
    }

    // Closes the game mode
    void close();

private:
    const GameContext& m_context;

    // Every game mode has a UI
    openglyph::ui::UserInterface m_ui;

    std::vector<CloseListener> m_close_listeners;
};

} // namespace openeaw