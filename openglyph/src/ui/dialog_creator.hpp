#pragma once

#include <khepri/ui/frame.hpp>
#include <khepri/ui/listbox.hpp>

#include <openglyph/ui/context.hpp>
#include <openglyph/ui/dialogs.hpp>

namespace openglyph::ui {

/**
 * This class creates widgets from dialog definitions
 */
class DialogCreator final
{
public:
    DialogCreator(Context& context);

    std::shared_ptr<khepri::ui::Widget> create_dialog_widget(const DialogDefinition& def,
                                                             DialogPosition          position);

private:
    std::shared_ptr<khepri::ui::Widget> create_widget(const PushButtonControlDefinition& def,
                                                      const khepri::Pointi&              offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const FrameControlDefinition& def,
                                                      const khepri::Pointi&         offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const LabelControlDefinition& def,
                                                      const khepri::Pointi&         offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const EditBoxControlDefinition& def,
                                                      const khepri::Pointi&           offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const ComboBoxControlDefinition& def,
                                                      const khepri::Pointi&            offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const ListBoxControlDefinition& def,
                                                      const khepri::Pointi&           offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const ScrollBarControlDefinition& def,
                                                      const khepri::Pointi&             offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const CheckBoxControlDefinition& def,
                                                      const khepri::Pointi&            offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const RadioButtonControlDefinition& def,
                                                      const khepri::Pointi&               offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const TrackBarControlDefinition& def,
                                                      const khepri::Pointi&            offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const ProgressBarControlDefinition& def,
                                                      const khepri::Pointi&               offset);
    std::shared_ptr<khepri::ui::Widget> create_widget(const ImageControlDefinition& def,
                                                      const khepri::Pointi&         offset);

private:
    khepri::ui::Frame::Style     dialog_style(std::string_view dialog_name) const;
    khepri::ui::Frame::Style     frame_style(std::string_view control_name) const;
    std::u16string_view          load_string(std::string_view key) const;
    khepri::ui::Listbox::Style   listbox_style(std::string_view control_name) const;
    khepri::ui::Renderer::FontId get_font(const StyleStore::FontStyle& style) const;

    Context& m_context;
};

} // namespace openglyph::ui