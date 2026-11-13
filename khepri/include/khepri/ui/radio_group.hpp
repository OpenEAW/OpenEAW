#pragma once

#include "radio_button.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace khepri::ui {

/**
 * A group for radio buttons.
 *
 * A radio group ensures that only one of the multiple attached radio buttons is selected. It can be
 * used to query the logical selection of the multiple radio buttons.
 */
class RadioGroup
{
public:
    /// Listener for selection change events on the button
    using SelectionListener = std::function<void()>;

    /**
     * Attaches a radio button to the group.
     *
     * \param[in] radio_button the radio button to attach
     *
     * After attaching, the radio group will track the selected state of the button and update the
     * group's selection (and attached button's state) if it changes.
     * If the button is already selected, this immediately affects the selection of the radio group.
     *
     * This method does nothing if the button is already attached.
     *
     * \throws khepri::argument_error if \a radio_button is null.
     */
    void attach(std::shared_ptr<RadioButton> radio_button);

    /**
     * Detaches a radio button from the group.
     *
     * \param[in] radio_button the radio button to detach
     *
     * Afterwards, the radio group will no longer track the selected state of the button. Any change
     * in the selected state of the button no longer affects the radio group's selection state or
     * other attached radio buttons. If the detached button was the selected button in the radio
     * group, the group's selection is cleared.
     *
     * This method does nothing if the button is not attached.
     */
    void detach(std::shared_ptr<RadioButton> radio_button);

    /**
     * Returns the currently selected radio button.
     *
     * Returns null if no radio button is selected.
     */
    std::shared_ptr<RadioButton> selection() const noexcept
    {
        return m_selection;
    }

    /**
     * Adds a selection change listener to the button.
     *
     * \param[in] listener the state change listener to add.
     *
     * Selection change listeners are called when a different radio button attached to this radio
     * group has been selected.
     */
    void add_selection_listener(const SelectionListener& listener);

    /**
     * Sets the radio group's selection
     *
     * \param[in] radio_button the new selection of the group (can be null)
     *
     * Updates each of the attached radio buttons selected state to reflect the new group
     * selection and calls the selections listener if the selection has changed.
     *
     * Does nothing if the radio button is not attached to the group.
     */
    void select(RadioButton* radio_button);

    /// \copydoc select(RadioButton*)
    void select(std::shared_ptr<RadioButton> radio_button);

private:
    std::vector<SelectionListener>                                         m_selection_listeners;
    std::vector<std::pair<std::shared_ptr<RadioButton>, ScopedConnection>> m_radio_buttons;

    std::shared_ptr<RadioButton> m_selection;
};

} // namespace khepri::ui
