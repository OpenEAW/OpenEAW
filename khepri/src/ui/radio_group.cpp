#include <khepri/exceptions.hpp>
#include <khepri/ui/radio_group.hpp>

namespace khepri::ui {

void RadioGroup::attach(std::shared_ptr<RadioButton> radio_button)
{
    if (radio_button == nullptr) {
        throw ArgumentError();
    }

    if (std::find_if(m_radio_buttons.begin(), m_radio_buttons.end(), [&](const auto& pair) {
            return pair.first == radio_button;
        }) == m_radio_buttons.end()) {
        // Capture raw pointer in the lambda to avoid a circular reference
        auto conn = radio_button->add_state_listener([this, rb = radio_button.get()] {
            if (rb->selected()) {
                select(rb);
            }
        });

        m_radio_buttons.push_back({radio_button, std::move(conn)});

        if (radio_button->selected()) {
            select(radio_button);
        }
    }
}

void RadioGroup::detach(std::shared_ptr<RadioButton> radio_button)
{
    auto it = std::find_if(m_radio_buttons.begin(), m_radio_buttons.end(),
                           [&](const auto& pair) { return pair.first == radio_button; });
    if (it != m_radio_buttons.end()) {
        m_radio_buttons.erase(it);
        if (m_selection == radio_button) {
            select(nullptr);
        }
    }
}

void RadioGroup::add_selection_listener(const SelectionListener& listener)
{
    m_selection_listeners.push_back(listener);
}

void RadioGroup::select(RadioButton* radio_button)
{
    std::shared_ptr<RadioButton> radio_button_ptr;
    if (radio_button != nullptr) {
        auto it = std::find_if(m_radio_buttons.begin(), m_radio_buttons.end(),
                               [&](const auto& pair) { return pair.first.get() == radio_button; });
        if (it == m_radio_buttons.end()) {
            // Radio button not part of the group
            return;
        }
        radio_button_ptr = it->first;
    }

    if (m_selection != radio_button_ptr) {
        m_selection = radio_button_ptr;

        for (const auto& pair : m_radio_buttons) {
            pair.first->selected(pair.first == m_selection);
        }

        for (const auto& listener : m_selection_listeners) {
            listener();
        }
    }
}

void RadioGroup::select(std::shared_ptr<RadioButton> radio_button)
{
    select(radio_button.get());
}

} // namespace khepri::ui
