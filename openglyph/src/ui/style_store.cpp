#include <khepri/log/log.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/parser/parsers.hpp>
#include <openglyph/ui/style_store.hpp>

using namespace std::literals;

namespace openglyph::ui {
namespace {
constexpr khepri::log::Logger LOG("ui");
} // namespace

StyleStore::TextureData StyleStore::parse_textures(const openglyph::XmlParser::Node& textures_node)
{
// Until C++ has P0149 (generalized member pointers, we need to work via a lambda)
#define SETTER(field) [](ControlTextures& cs, std::string_view str) { cs.field = std::string(str); }

    static const std::unordered_multimap<std::string_view,
                                         void (*)(ControlTextures&, std::string_view)>
        style_setters = {
            // Dialog
            {"Frame_Bottom"sv, SETTER(dialog_textures.bottom_texture)},
            {"Frame_Bottom_Left"sv, SETTER(dialog_textures.bottom_left_texture)},
            {"Frame_Bottom_Right"sv, SETTER(dialog_textures.bottom_right_texture)},
            {"Frame_Background"sv, SETTER(dialog_textures.background_texture)},
            {"Frame_Left"sv, SETTER(dialog_textures.left_texture)},
            {"Frame_Right"sv, SETTER(dialog_textures.right_texture)},
            {"Frame_Top"sv, SETTER(dialog_textures.top_texture)},
            {"Frame_Top_Left"sv, SETTER(dialog_textures.top_left_texture)},
            {"Frame_Top_Right"sv, SETTER(dialog_textures.top_right_texture)},
            {"Frame_Top_Transition_Left"sv, SETTER(dialog_textures.top_transition_left_texture)},
            {"Frame_Top_Transition_Right"sv, SETTER(dialog_textures.top_transition_right_texture)},
            {"Frame_Bottom_Transition_Left"sv,
             SETTER(dialog_textures.bottom_transition_left_texture)},
            {"Frame_Bottom_Transition_Right"sv,
             SETTER(dialog_textures.bottom_transition_right_texture)},
            {"Frame_Left_Transition_Top"sv, SETTER(dialog_textures.left_transition_top_texture)},
            {"Frame_Left_Transition_Bottom"sv,
             SETTER(dialog_textures.left_transition_bottom_texture)},
            {"Frame_Right_Transition_Top"sv, SETTER(dialog_textures.right_transition_top_texture)},
            {"Frame_Right_Transition_Bottom"sv,
             SETTER(dialog_textures.right_transition_bottom_texture)},

            // Frame
            {"Small_Frame_Bottom"sv, SETTER(frame_textures.bottom_texture)},
            {"Small_Frame_Bottom_Left"sv, SETTER(frame_textures.bottom_left_texture)},
            {"Small_Frame_Bottom_Right"sv, SETTER(frame_textures.bottom_right_texture)},
            {"Small_Frame_Left"sv, SETTER(frame_textures.left_texture)},
            {"Small_Frame_Right"sv, SETTER(frame_textures.right_texture)},
            {"Small_Frame_Top"sv, SETTER(frame_textures.top_texture)},
            {"Small_Frame_Top_Left"sv, SETTER(frame_textures.top_left_texture)},
            {"Small_Frame_Top_Right"sv, SETTER(frame_textures.top_right_texture)},
            {"Small_Frame_Background"sv, SETTER(frame_textures.background_texture)},

            // Button
            {"Button_Left"sv, SETTER(button_textures.normal.left_texture)},
            {"Button_Middle"sv, SETTER(button_textures.normal.middle_texture)},
            {"Button_Right"sv, SETTER(button_textures.normal.right_texture)},
            {"Button_Left_Mouse_Over"sv, SETTER(button_textures.mouse_over.left_texture)},
            {"Button_Middle_Mouse_Over"sv, SETTER(button_textures.mouse_over.middle_texture)},
            {"Button_Right_Mouse_Over"sv, SETTER(button_textures.mouse_over.right_texture)},
            {"Button_Left_Pressed"sv, SETTER(button_textures.pressed.left_texture)},
            {"Button_Middle_Pressed"sv, SETTER(button_textures.pressed.middle_texture)},
            {"Button_Right_Pressed"sv, SETTER(button_textures.pressed.right_texture)},
            {"Button_Left_Disabled"sv, SETTER(button_textures.disabled.left_texture)},
            {"Button_Middle_Disabled"sv, SETTER(button_textures.disabled.middle_texture)},
            {"Button_Right_Disabled"sv, SETTER(button_textures.disabled.right_texture)},

            // Checkbox
            {"Check_Off"sv, SETTER(checkbox_textures.off_texture)},
            {"Check_On"sv, SETTER(checkbox_textures.on_texture)},

            // Radio button
            {"Radio_Off"sv, SETTER(radiobutton_textures.off_texture)},
            {"Radio_On"sv, SETTER(radiobutton_textures.on_texture)},
            {"Radio_Disabled"sv, SETTER(radiobutton_textures.disabled_texture)},
            {"Radio_Mouse_Over"sv, SETTER(radiobutton_textures.mouse_over_texture)},

            // Scroll bar
            {"Scroll_Middle"sv, SETTER(scrollbar_textures.normal.middle_texture)},
            {"Scroll_Tab"sv, SETTER(scrollbar_textures.normal.tab_texture)},
            {"Scroll_Middle_Disabled"sv, SETTER(scrollbar_textures.disabled.middle_texture)},
            {"Scroll_Tab_Disabled"sv, SETTER(scrollbar_textures.disabled.tab_texture)},
            {"Scroll_Middle"sv, SETTER(scrollbar_textures.normal.middle_texture)},
            {"Scroll_Tab"sv, SETTER(scrollbar_textures.normal.tab_texture)},
            {"Scroll_Middle_Disabled"sv, SETTER(scrollbar_textures.disabled.middle_texture)},
            {"Scroll_Tab_Disabled"sv, SETTER(scrollbar_textures.disabled.tab_texture)},

            {"Scroll_Up_Button"sv, SETTER(scrollbar_textures.decrease_button.normal_texture)},
            {"Scroll_Up_Button_Pressed"sv,
             SETTER(scrollbar_textures.decrease_button.pressed_texture)},
            {"Scroll_Up_Button_Mouse_Over"sv,
             SETTER(scrollbar_textures.decrease_button.mouse_over_texture)},
            {"Scroll_Up_Button_Disabled"sv,
             SETTER(scrollbar_textures.decrease_button.disabled_texture)},
            {"Scroll_Down_Button"sv, SETTER(scrollbar_textures.increase_button.normal_texture)},
            {"Scroll_Down_Button_Pressed"sv,
             SETTER(scrollbar_textures.increase_button.pressed_texture)},
            {"Scroll_Down_Button_Mouse_Over"sv,
             SETTER(scrollbar_textures.increase_button.mouse_over_texture)},
            {"Scroll_Down_Button_Disabled"sv,
             SETTER(scrollbar_textures.increase_button.disabled_texture)},

            // Horizontal TrackBar
            {"Dial_Plus"sv, SETTER(trackbar_horz_textures.increase_button.normal_texture)},
            {"Dial_Plus_Mouse_Over"sv,
             SETTER(trackbar_horz_textures.increase_button.mouse_over_texture)},
            {"Dial_Plus_Pressed"sv, SETTER(trackbar_horz_textures.increase_button.pressed_texture)},
            {"Dial_Plus_Disabled"sv,
             SETTER(trackbar_horz_textures.increase_button.disabled_texture)},
            {"Dial_Minus"sv, SETTER(trackbar_horz_textures.decrease_button.normal_texture)},
            {"Dial_Minus_Mouse_Over"sv,
             SETTER(trackbar_horz_textures.decrease_button.mouse_over_texture)},
            {"Dial_Minus_Pressed"sv,
             SETTER(trackbar_horz_textures.decrease_button.pressed_texture)},
            {"Dial_Minus_Disabled"sv,
             SETTER(trackbar_horz_textures.decrease_button.disabled_texture)},
            {"Dial_Left"sv, SETTER(trackbar_horz_textures.normal.start_texture)},
            {"Dial_Middle"sv, SETTER(trackbar_horz_textures.normal.middle_texture)},
            {"Dial_Right"sv, SETTER(trackbar_horz_textures.normal.end_texture)},
            {"Dial_Tab"sv, SETTER(trackbar_horz_textures.normal.tab_texture)},
            {"Dial_Left_Disabled"sv, SETTER(trackbar_horz_textures.disabled.start_texture)},
            {"Dial_Middle_Disabled"sv, SETTER(trackbar_horz_textures.disabled.middle_texture)},
            {"Dial_Right_Disabled"sv, SETTER(trackbar_horz_textures.disabled.end_texture)},
            {"Dial_Tab_Disabled"sv, SETTER(trackbar_horz_textures.disabled.tab_texture)},

            // Vertical TrackBar
            {"Trackbar_Scroll_Down_Button"sv,
             SETTER(trackbar_vert_textures.decrease_button.normal_texture)},
            {"Trackbar_Scroll_Down_Button_Pressed"sv,
             SETTER(trackbar_vert_textures.decrease_button.mouse_over_texture)},
            {"Trackbar_Scroll_Down_Button_Mouse_Over"sv,
             SETTER(trackbar_vert_textures.decrease_button.pressed_texture)},
            {"Trackbar_Scroll_Down_Button_Disabled"sv,
             SETTER(trackbar_vert_textures.decrease_button.disabled_texture)},
            {"Trackbar_Scroll_Up_Button"sv,
             SETTER(trackbar_vert_textures.increase_button.normal_texture)},
            {"Trackbar_Scroll_Up_Button_Pressed"sv,
             SETTER(trackbar_vert_textures.increase_button.mouse_over_texture)},
            {"Trackbar_Scroll_Up_Button_Mouse_Over"sv,
             SETTER(trackbar_vert_textures.increase_button.pressed_texture)},
            {"Trackbar_Scroll_Up_Button_Disabled"sv,
             SETTER(trackbar_vert_textures.increase_button.disabled_texture)},
            {"Trackbar_Scroll_Middle"sv, SETTER(trackbar_vert_textures.normal.middle_texture)},
            {"Trackbar_Scroll_Tab"sv, SETTER(trackbar_vert_textures.normal.tab_texture)},
            {"Trackbar_Scroll_Middle_Disabled"sv,
             SETTER(trackbar_vert_textures.disabled.middle_texture)},
            {"Trackbar_Scroll_Tab_Disabled"sv, SETTER(trackbar_vert_textures.disabled.tab_texture)},

            // Combo box
            {"Combo_Box_Popdown_Button"sv, SETTER(combobox_textures.popdown_button_texture)},
            {"Combo_Box_Popdown_Button_Mouse_Over"sv,
             SETTER(combobox_textures.popdown_button_mouse_over_texture)},
            {"Combo_Box_Popdown_Button_Pressed"sv,
             SETTER(combobox_textures.popdown_button_pressed_texture)},
            {"Combo_Box_Text_Box"sv, SETTER(combobox_textures.text_box_texture)},
            {"Combo_Box_Left_Cap"sv, SETTER(combobox_textures.left_cap_texture)},

            // Progress bar
            {"Progress_Bar_Left"sv, SETTER(progressbar_textures.left_texture)},
            {"Progress_Bar_Middle_Off"sv, SETTER(progressbar_textures.middle_off_texture)},
            {"Progress_Bar_Middle_On"sv, SETTER(progressbar_textures.middle_on_texture)},
            {"Progress_Bar_Right"sv, SETTER(progressbar_textures.right_texture)},

            // Image
            {"Scanlines"sv, SETTER(image_textures.scanlines_texture)},
        };

#undef SETTER

    StyleStore::TextureData textures;

    if (const auto default_node = textures_node.child("Default")) {
        // Read the default textures
        for (const auto& texture_node : default_node->nodes()) {
            if (auto it = style_setters.find(texture_node.name()); it != style_setters.end()) {
                (*it->second)(textures.default_textures, texture_node.value());
            }
        }
    }

    // Read overrides for special controls
    for (const auto& control_node : textures_node.nodes()) {
        if (!khepri::case_insensitive_equals(control_node.name(), "Default")) {
            auto [control_it, inserted] =
                textures.override_textures.emplace(control_node.name(), textures.default_textures);

            for (const auto& texture_node : control_node.nodes()) {
                if (auto it = style_setters.find(texture_node.name()); it != style_setters.end()) {
                    (*it->second)(control_it->second, texture_node.value());
                }
            }
        }
    }

    return textures;
}

template <typename T>
void read_child(const openglyph::XmlParser::Node& node, std::string_view attr_name, T& output)
{
    if (const auto child = node.child(attr_name)) {
        output = parse<T>(child->value());
    }
}

void parse_font(StyleStore::FontStyle& font, const openglyph::XmlParser::Node& font_node)
{
    khepri::ColorSRGBA color_top;
    khepri::ColorSRGBA color_bottom;
    read_child(font_node, "Name", font.name);
    read_child(font_node, "Size", font.point_size);
    read_child(font_node, "Top_Color", color_top);
    read_child(font_node, "Bottom_Color", color_bottom);
    read_child(font_node, "Emboss", font.has_emboss);
    read_child(font_node, "Outline", font.has_stroke);
    read_child(font_node, "Character_Padding", font.char_spacing);
    read_child(font_node, "Stretch_Factor", font.stretch_factor);

    // Glyph uses DX9 and has no knowledge of sRGB space. All sRGB-style colors in the configuration
    // are actually in linear space.
    font.color_top    = khepri::ColorRGBA{color_top.r / 255.0f, color_top.g / 255.0f,
                                       color_top.b / 255.0f, color_top.a / 255.0f};
    font.color_bottom = khepri::ColorRGBA{color_bottom.r / 255.0f, color_bottom.g / 255.0f,
                                          color_bottom.b / 255.0f, color_bottom.a / 255.0f};
}

StyleStore::FontData StyleStore::parse_fonts(const openglyph::XmlParser::Node& fonts_node)
{
    FontData fonts;

    StyleStore::FontStyle default_font{};
    if (const auto default_node = fonts_node.child("Default")) {
        // Read the default fonts
        static const std::unordered_multimap<std::string_view, FontControlType> control_types = {
            {"Push_Button", FontControlType::Button},
            {"List_Box", FontControlType::ListBox},
            {"Combo_Box", FontControlType::ComboBox},
            {"Edit_Box", FontControlType::EditBox},
            {"L_Text", FontControlType::Label},
            {"Overlay_Caption_Text", FontControlType::Image},
        };

        if (const auto global_default_node = default_node->child("Global_Default")) {
            parse_font(default_font, *global_default_node);
            for (unsigned int i = 0; i < NUM_CONTROL_TYPES; ++i) {
                fonts.default_fonts[i] = default_font;
            }
        }

        for (const auto& control_node : default_node->nodes()) {
            if (auto it = control_types.find(control_node.name()); it != control_types.end()) {
                parse_font(fonts.default_fonts[static_cast<unsigned int>(it->second)],
                           control_node);
            }
        }
    }

    // Read overrides for special controls
    for (const auto& control_node : fonts_node.nodes()) {
        if (!khepri::case_insensitive_equals(control_node.name(), "Default")) {
            auto control_font = default_font;
            parse_font(control_font, control_node);
            fonts.override_fonts.emplace(control_node.name(), std::move(control_font));
        }
    }

    return fonts;
}

StyleStore::StyleStore(AssetLoader& asset_loader, std::string_view xml_filename)
{
    try {
        if (auto xml_stream = asset_loader.open_config(xml_filename)) {
            const openglyph::XmlParser xml(*xml_stream);
            if (const auto& root = xml.root()) {
                if (const auto textures_node = root->child("Textures")) {
                    m_mega_texture_filename = optional_attribute(*textures_node, "File", "");
                    m_textures              = parse_textures(*textures_node);
                }
                if (const auto fonts_node = root->child("Fonts")) {
                    m_fonts = parse_fonts(*fonts_node);
                }
            }
        }
    } catch (const openglyph::ParseError& e) {
        LOG.error("parse error: {}", e.what());
    }
}

template <typename TextureType>
const TextureType&
StyleStore::control_textures(std::string_view  control_name,
                             const TextureType ControlTextures::* control_texturess) const noexcept
{
    // Try to find an override for the specific control
    if (auto it = m_textures.override_textures.find(control_name);
        it != m_textures.override_textures.end()) {
        return it->second.*control_texturess;
    }
    // Not found, return the default
    return m_textures.default_textures.*control_texturess;
}

const StyleStore::FontStyle& StyleStore::control_font(std::string_view control_name,
                                                      FontControlType  control_type) const noexcept
{
    // Try to find an override for the specific control
    if (auto it = m_fonts.override_fonts.find(control_name); it != m_fonts.override_fonts.end()) {
        return it->second;
    }
    // Not found, return the default
    return m_fonts.default_fonts[static_cast<unsigned int>(control_type)];
}

const StyleStore::DialogTextures&
StyleStore::dialog_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::dialog_textures);
}

const StyleStore::FrameTextures&
StyleStore::frame_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::frame_textures);
}

const StyleStore::ButtonTextures&
StyleStore::button_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::button_textures);
}

const StyleStore::CheckboxTextures&
StyleStore::checkbox_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::checkbox_textures);
}

const StyleStore::RadioButtonTextures&
StyleStore::radiobutton_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::radiobutton_textures);
}

const StyleStore::TrackbarTextures& StyleStore::trackbar_textures(std::string_view control_name,
                                                                  bool is_vertical) const noexcept
{
    return control_textures(control_name, &ControlTextures::trackbar_vert_textures);
}

const StyleStore::ScrollBarTextures&
StyleStore::scrollbar_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::scrollbar_textures);
}

const StyleStore::ComboBoxTextures&
StyleStore::combobox_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::combobox_textures);
}

const StyleStore::ProgressBarTextures&
StyleStore::progressbar_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::progressbar_textures);
}

const StyleStore::ImageTextures&
StyleStore::image_textures(std::string_view control_name) const noexcept
{
    return control_textures(control_name, &ControlTextures::image_textures);
}

const StyleStore::FontStyle& StyleStore::button_font(std::string_view control_name) const noexcept
{
    return control_font(control_name, FontControlType::Button);
}

const StyleStore::FontStyle& StyleStore::listbox_font(std::string_view control_name) const noexcept
{
    return control_font(control_name, FontControlType::ListBox);
}

const StyleStore::FontStyle& StyleStore::combobox_font(std::string_view control_name) const noexcept
{
    return control_font(control_name, FontControlType::ComboBox);
}

const StyleStore::FontStyle& StyleStore::editbox_font(std::string_view control_name) const noexcept
{
    return control_font(control_name, FontControlType::EditBox);
}

const StyleStore::FontStyle& StyleStore::label_font(std::string_view control_name) const noexcept
{
    return control_font(control_name, FontControlType::Label);
}

const StyleStore::FontStyle& StyleStore::image_font(std::string_view control_name) const noexcept
{
    return control_font(control_name, FontControlType::Image);
}

} // namespace openglyph::ui