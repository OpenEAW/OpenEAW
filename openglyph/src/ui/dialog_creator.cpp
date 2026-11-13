#include "dialog_creator.hpp"

#include <khepri/ui/button.hpp>
#include <khepri/ui/combobox.hpp>
#include <khepri/ui/label.hpp>
#include <khepri/utility/string.hpp>

namespace openglyph::ui {
namespace {

template <typename LayoutType = khepri::ui::Layout>
khepri::ui::Layout make_layout(const DialogPosition position, const khepri::Size& dialog_size)
{
    const auto width  = static_cast<long>(dialog_size.width);
    const auto height = static_cast<long>(dialog_size.height);
    switch (position) {
    default:
    case DialogPosition::center:
        return khepri::ui::Layout::center(dialog_size);
    case DialogPosition::top_left:
        return khepri::ui::Layout::top_left({0, 0}, dialog_size);
    case DialogPosition::top_middle:
        return khepri::ui::Layout::top_middle({-width / 2, 0}, dialog_size);
    case DialogPosition::top_right:
        return khepri::ui::Layout::top_right({-width, 0}, dialog_size);
    case DialogPosition::bottom_left:
        return khepri::ui::Layout::bottom_left({0, -height}, dialog_size);
    case DialogPosition::bottom_middle:
        return khepri::ui::Layout::bottom_middle({-width / 2, -height}, dialog_size);
    case DialogPosition::bottom_right:
        return khepri::ui::Layout::bottom_right({-width, -height}, dialog_size);
    case DialogPosition::left_middle:
        return khepri::ui::Layout::left_middle({0, -height / 2}, dialog_size);
    case DialogPosition::right_middle:
        return khepri::ui::Layout::right_middle({-width, -height / 2}, dialog_size);
    case DialogPosition::full_screen:
        return khepri::ui::Layout::fill();
    }
    return {};
}

template <typename LayoutType = khepri::ui::Layout>
LayoutType make_layout(const CommonControlDefinition& def, const khepri::Pointi& offset) noexcept
{
    LayoutType layout{};
    static_cast<khepri::ui::Layout&>(layout) = khepri::ui::Layout::top_left(
        {def.position.x + offset.x, def.position.y + offset.y}, def.size);
    return layout;
}
} // namespace

DialogCreator::DialogCreator(Context& context) : m_context(context) {}

std::shared_ptr<khepri::ui::Widget> DialogCreator::create_dialog_widget(const DialogDefinition& def,
                                                                        DialogPosition position)
{
    const auto style = dialog_style(def.name);

    // For dialogs, the frame is 'external', so we have to expand the size to account for the frame
    // borders, and then move all controls by the same amount to keep them in the same position.
    auto                 size = def.size;
    const khepri::Pointi offset{static_cast<long>(style.left_slice.tex_rect.width),
                                static_cast<long>(style.top_slice.tex_rect.height)};
    size.width += offset.x + style.right_slice.tex_rect.width;
    size.height += offset.y + style.bottom_slice.tex_rect.height;

    auto frame = std::make_shared<khepri::ui::Frame>(make_layout(position, size));

    frame->style(style);

    for (const auto& control_def : def.controls) {
        if (auto widget = std::visit(
                [&, this](const auto& def) { return create_widget(def, offset); }, control_def)) {
            frame->add(std::move(widget));
        }
    }
    return frame;
}

std::shared_ptr<khepri::ui::Widget> DialogCreator::create_widget(const LabelControlDefinition& def,
                                                                 const khepri::Pointi& offset)
{
    const auto label = std::make_shared<khepri::ui::Label>(make_layout(def, offset));
    label->name(def.name);
    label->text(load_string(def.text));
    label->style({m_context.sprite_material_id(),
                  get_font(m_context.style_store().label_font(def.name)), def.text_alignment});
    return label;
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const PushButtonControlDefinition& def, const khepri::Pointi& offset)
{
    const auto& textures = m_context.style_store().button_textures(def.name);
    const auto  font_id  = get_font(m_context.style_store().button_font(def.name));

    const auto& state_style = [&](const auto& textures) {
        const auto left_texture   = m_context.get_texture(textures.left_texture);
        const auto middle_texture = m_context.get_texture(textures.middle_texture);
        const auto right_texture  = m_context.get_texture(textures.right_texture);

        khepri::ui::Button::Style::State state{
            {{left_texture.tex_rect.width, 0, right_texture.tex_rect.width, 0}, {}},
            {m_context.sprite_material_id(), font_id, def.text_alignment}};
        state.frame.left_slice   = left_texture;
        state.frame.middle_slice = middle_texture;
        state.frame.right_slice  = right_texture;
        return state;
    };

    const khepri::ui::Button::Style style{
        state_style(textures.normal), state_style(textures.disabled),
        state_style(textures.mouse_over), state_style(textures.pressed)};

    // Update the layout to make the height equal to the middle texture's height.
    // The texture height is leading in Glyph.
    auto layout = make_layout(def, offset);
    assert(layout.top.parent_frac == 0.0);
    layout.bottom = {
        0, static_cast<long>(layout.top.offset + style.enabled.frame.middle_slice.tex_rect.height)};

    const auto button = std::make_shared<khepri::ui::Button>(layout);

    button->name(def.name);
    button->text(load_string(def.text));
    button->style(style);
    return button;
}

std::shared_ptr<khepri::ui::Widget> DialogCreator::create_widget(const FrameControlDefinition& def,
                                                                 const khepri::Pointi& offset)
{
    const auto frame = std::make_shared<khepri::ui::Frame>(make_layout(def, offset));
    auto       style = frame_style(def.name);

    // Frame controls don't have a background, just a frame
    style.background_slice = {};

    frame->name(def.name);
    frame->style(style);
    return frame;
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const EditBoxControlDefinition& def, const khepri::Pointi& offset)
{
    // TODO
    return {};
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const ComboBoxControlDefinition& def, const khepri::Pointi& offset)
{
    const auto& textures = m_context.style_store().combobox_textures(def.name);
    const auto  font_id  = get_font(m_context.style_store().combobox_font(def.name));

    const auto state_style = [&](const auto& texture) {
        khepri::ui::Button::Style::State state{};
        state.frame.background_slice = m_context.get_texture(texture);
        state.frame.material         = m_context.sprite_material_id();
        return state;
    };

    khepri::ui::Combobox::Style style{};
    style.dropdown_button.enabled   = state_style(textures.popdown_button_texture);
    style.dropdown_button.disabled  = state_style(textures.popdown_button_texture);
    style.dropdown_button.mouseover = state_style(textures.popdown_button_mouse_over_texture);
    style.dropdown_button.pressed   = state_style(textures.popdown_button_pressed_texture);
    style.dropdown_listbox          = listbox_style("Child Listbox");
    style.label.material            = m_context.sprite_material_id();
    style.label.font                = get_font(m_context.style_store().listbox_font(def.name));
    style.label.text_align          = khepri::ui::TextAlign::center;

    // FoC uses dedicated "left cap" and "text box" textures for the combobox. EaW uses the
    // frame style instead. Check if we have either of the FoC textures; if so, use FoC
    // behavior, otherwise, use EaW behavior.
    if (!textures.left_cap_texture.empty() || !textures.text_box_texture.empty()) {
        style.frame.left_slice   = m_context.get_texture(textures.left_cap_texture);
        style.frame.middle_slice = m_context.get_texture(textures.text_box_texture);
        style.frame.margins.left = style.frame.left_slice.tex_rect.width;
        style.frame.material     = m_context.sprite_material_id();
    } else {
        style.frame = frame_style(def.name);
    }

    auto layout = make_layout<khepri::ui::Combobox::Layout>(def, offset);
    // Combobox height is not taken from the resource definition, but determined by the texture
    // heights
    layout.bottom = {
        0, layout.top.offset +
               static_cast<long>(
                   std::max(std::max(style.frame.left_slice.tex_rect.height,
                                     style.frame.background_slice.tex_rect.height),
                            style.dropdown_button.enabled.frame.background_slice.tex_rect.height))};
    layout.dropdown_button_width = static_cast<unsigned long>(
        style.dropdown_button.enabled.frame.background_slice.tex_rect.width);

    layout.dropdown_scrollbar_width = style.dropdown_listbox.scroll_panel.scrollbar.button_track
                                          .enabled.frame.background_slice.tex_rect.width;
    layout.dropdown_updown_button_height =
        std::max(style.dropdown_listbox.scroll_panel.scrollbar.button_up.enabled.frame
                     .background_slice.tex_rect.height,
                 style.dropdown_listbox.scroll_panel.scrollbar.button_down.enabled.frame
                     .background_slice.tex_rect.height);
    layout.dropdown_track_button_size = {style.dropdown_listbox.scroll_panel.scrollbar.button_track
                                             .enabled.frame.background_slice.tex_rect.width,
                                         style.dropdown_listbox.scroll_panel.scrollbar.button_track
                                             .enabled.frame.background_slice.tex_rect.height};

    const auto combobox = std::make_shared<khepri::ui::Combobox>(layout);
    combobox->name(def.name);
    combobox->style(style);
    return combobox;
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const ListBoxControlDefinition& def, const khepri::Pointi& offset)
{
    auto style = listbox_style(def.name);

    auto layout = make_layout<khepri::ui::Listbox::Layout>(def, offset);
    layout.scrollbar_width =
        style.scroll_panel.scrollbar.button_track.enabled.frame.background_slice.tex_rect.width;
    layout.updown_button_height = std::max(
        style.scroll_panel.scrollbar.button_up.enabled.frame.background_slice.tex_rect.height,
        style.scroll_panel.scrollbar.button_down.enabled.frame.background_slice.tex_rect.height);
    layout.track_button_size = {
        style.scroll_panel.scrollbar.button_track.enabled.frame.background_slice.tex_rect.width,
        style.scroll_panel.scrollbar.button_track.enabled.frame.background_slice.tex_rect.height};

    const auto listbox = std::make_shared<khepri::ui::Listbox>(layout);
    listbox->name(def.name);
    listbox->style(style);
    return listbox;
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const ScrollBarControlDefinition& def, const khepri::Pointi& offset)
{
    // TODO
    return {};
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const CheckBoxControlDefinition& de, const khepri::Pointi& offset)
{
    // TODO
    return {};
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const RadioButtonControlDefinition& def, const khepri::Pointi& offset)
{
    // TODO
    return {};
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const TrackBarControlDefinition& def, const khepri::Pointi& offset)
{
    // TODO
    return {};
}

std::shared_ptr<khepri::ui::Widget>
DialogCreator::create_widget(const ProgressBarControlDefinition& def, const khepri::Pointi& offset)
{
    // TODO
    return {};
}

std::shared_ptr<khepri::ui::Widget> DialogCreator::create_widget(const ImageControlDefinition& def,
                                                                 const khepri::Pointi& offset)
{
    // TODO
    return {};
}

khepri::ui::Frame::Style DialogCreator::dialog_style(std::string_view dialog_name) const
{
    const auto& textures = m_context.style_store().dialog_textures(dialog_name);

    khepri::ui::Frame::Style style;

    // The 'background' texture is actually placed between the frames for dialogs
    style.middle_slice = m_context.get_texture(textures.background_texture);

    style.top_left_slice     = m_context.get_texture(textures.top_left_texture);
    style.top_first_slice    = m_context.get_texture(textures.top_transition_left_texture);
    style.top_slice          = m_context.get_texture(textures.top_texture);
    style.top_last_slice     = m_context.get_texture(textures.top_transition_right_texture);
    style.top_right_slice    = m_context.get_texture(textures.top_right_texture);
    style.right_above_slice  = m_context.get_texture(textures.right_transition_top_texture);
    style.right_slice        = m_context.get_texture(textures.right_texture);
    style.right_below_slice  = m_context.get_texture(textures.right_transition_bottom_texture);
    style.bottom_right_slice = m_context.get_texture(textures.bottom_right_texture);
    style.bottom_first_slice = m_context.get_texture(textures.bottom_transition_left_texture);
    style.bottom_slice       = m_context.get_texture(textures.bottom_texture);
    style.bottom_last_slice  = m_context.get_texture(textures.bottom_transition_right_texture);
    style.bottom_left_slice  = m_context.get_texture(textures.bottom_left_texture);
    style.left_below_slice   = m_context.get_texture(textures.left_transition_bottom_texture);
    style.left_slice         = m_context.get_texture(textures.left_texture);
    style.left_above_slice   = m_context.get_texture(textures.left_transition_top_texture);

    // Margins are determined by the corner slices
    style.margins = {style.top_left_slice.tex_rect.width, style.top_left_slice.tex_rect.height,
                     style.bottom_right_slice.tex_rect.width,
                     style.bottom_right_slice.tex_rect.height};

    style.material = m_context.sprite_material_id();
    return style;
}

khepri::ui::Frame::Style DialogCreator::frame_style(std::string_view control_name) const
{
    const auto& textures = m_context.style_store().frame_textures(control_name);

    khepri::ui::Frame::Style style;
    style.background_slice   = m_context.get_texture(textures.background_texture);
    style.bottom_slice       = m_context.get_texture(textures.bottom_texture);
    style.bottom_left_slice  = m_context.get_texture(textures.bottom_left_texture);
    style.bottom_right_slice = m_context.get_texture(textures.bottom_right_texture);
    style.left_slice         = m_context.get_texture(textures.left_texture);
    style.right_slice        = m_context.get_texture(textures.right_texture);
    style.top_slice          = m_context.get_texture(textures.top_texture);
    style.top_left_slice     = m_context.get_texture(textures.top_left_texture);
    style.top_right_slice    = m_context.get_texture(textures.top_right_texture);
    style.material           = m_context.sprite_material_id();
    style.margins            = {
        std::max(style.left_slice.tex_rect.width, std::max(style.top_left_slice.tex_rect.width,
                                                                      style.bottom_left_slice.tex_rect.width)),
        std::max(style.top_left_slice.tex_rect.height,
                            std::max(style.top_slice.tex_rect.height, style.top_right_slice.tex_rect.height)),
        std::max(style.right_slice.tex_rect.width,
                            std::max(style.top_right_slice.tex_rect.width,
                                     style.bottom_right_slice.tex_rect.width)),
        std::max(style.bottom_left_slice.tex_rect.height,
                            std::max(style.bottom_slice.tex_rect.height,
                                     style.bottom_right_slice.tex_rect.height))};
    return style;
}

khepri::ui::Listbox::Style DialogCreator::listbox_style(std::string_view control_name) const
{
    khepri::ui::Listbox::Style style;
    style.labels.material = m_context.sprite_material_id();
    style.labels.font     = get_font(m_context.style_store().listbox_font(control_name));

    const auto* font = m_context.ui_renderer().get_font(style.labels.font);

    style.line_height        = font->line_height();
    style.selection_material = m_context.rect_material_id();
    style.selection_color    = khepri::ColorRGBA(0.0f, 0.0f, 1.0, 0.3f);
    style.scroll_panel.frame = frame_style(control_name);

    const auto& scrollbar_textures = m_context.style_store().scrollbar_textures(control_name);

    const auto& button_style = [this](const StyleStore::ScrollBarTextures::Button& style) {
        khepri::ui::Button::Style bstyle;
        bstyle.enabled.frame.background_slice   = m_context.get_texture(style.normal_texture);
        bstyle.disabled.frame.background_slice  = m_context.get_texture(style.disabled_texture);
        bstyle.mouseover.frame.background_slice = m_context.get_texture(style.mouse_over_texture);
        bstyle.pressed.frame.background_slice   = m_context.get_texture(style.pressed_texture);

        bstyle.enabled.frame.material   = m_context.sprite_material_id();
        bstyle.disabled.frame.material  = m_context.sprite_material_id();
        bstyle.mouseover.frame.material = m_context.sprite_material_id();
        bstyle.pressed.frame.material   = m_context.sprite_material_id();
        return bstyle;
    };

    const auto& scroll_style = [this](const StyleStore::ScrollBarTextures::ScrollArea& style) {
        khepri::ui::Frame::Style fstyle;
        fstyle.top_slice    = m_context.get_texture(style.start_texture);
        fstyle.middle_slice = m_context.get_texture(style.middle_texture);
        fstyle.bottom_slice = m_context.get_texture(style.end_texture);
        fstyle.material     = m_context.sprite_material_id();
        fstyle.margins      = {0, fstyle.top_slice.tex_rect.height, 0,
                               fstyle.bottom_slice.tex_rect.height};
        return fstyle;
    };

    style.scroll_panel.scrollbar.button_up   = button_style(scrollbar_textures.decrease_button);
    style.scroll_panel.scrollbar.button_down = button_style(scrollbar_textures.increase_button);
    style.scroll_panel.scrollbar.frame_track_enabled  = scroll_style(scrollbar_textures.normal);
    style.scroll_panel.scrollbar.frame_track_disabled = scroll_style(scrollbar_textures.disabled);
    style.scroll_panel.scrollbar.button_track.enabled.frame.material =
        m_context.sprite_material_id();
    style.scroll_panel.scrollbar.button_track.enabled.frame.background_slice =
        m_context.get_texture(scrollbar_textures.normal.tab_texture);

    style.scroll_panel.scrollbar.button_track.pressed.frame =
        style.scroll_panel.scrollbar.button_track.enabled.frame;
    style.scroll_panel.scrollbar.button_track.mouseover.frame =
        style.scroll_panel.scrollbar.button_track.enabled.frame;

    style.scroll_panel.scrollbar.button_track.disabled.frame.material =
        m_context.sprite_material_id();
    style.scroll_panel.scrollbar.button_track.disabled.frame.background_slice =
        m_context.get_texture(scrollbar_textures.disabled.tab_texture);

    return style;
}

std::u16string_view DialogCreator::load_string(std::string_view key) const
{
    if (key.empty()) {
        // An empty key means "no string"
        return {};
    }

    if (auto str = m_context.string_database().get(key)) {
        return *str;
    }

    // The string wasn't found, so return a placeholder string.
    return u"[MISSING]";
}

khepri::ui::Renderer::FontId DialogCreator::get_font(const StyleStore::FontStyle& style) const
{
    // EaW uses several Arial fonts in various places, but it's not available on non-Windows
    // platforms and it's a licensed font so we can't ship it along with our programs. So instead we
    // silently replace it with an open-source font that's very similar.
    if (!khepri::starts_with(style.name, "Arial")) {
        return m_context.get_font(style);
    }

    auto new_style = style;
    new_style.name = "NotoSansDisplay-Regular";
    return m_context.get_font(new_style);
}

} // namespace openglyph::ui