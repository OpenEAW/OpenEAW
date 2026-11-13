#pragma once

#include <khepri/math/color_srgba.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/parser/xml_parser.hpp>

#include <any>
#include <map>
#include <string>
#include <string_view>

namespace openglyph::ui {

class StyleStore final
{
public:
    // Style for fonts
    struct FontStyle
    {
        /// Name of the font
        std::string name;

        /// Size of the font, in points (assuming 800x600, 96dpi)
        unsigned int point_size{8};

        /// Color at the top of the line's gradient
        khepri::ColorRGBA color_top{1, 1, 1, 1};

        /// Color at the bottom of the line's gradient
        khepri::ColorRGBA color_bottom{1, 1, 1, 1};

        /// Is the font embossed?
        /// Embossed fonts have top borders darkened and bottom borders brightened to make it seem
        /// like they are lit from the bottom.
        bool has_emboss{false};

        /// Does the font have a stroke (outline)?
        bool has_stroke{false};

        /// Stretch the font vertically by this factor.
        float stretch_factor{1.0f};

        /// How many pixels of spacing between characters?
        unsigned int char_spacing{0};
    };

    // Textures for dialogs
    struct DialogTextures
    {
        std::string top_left_texture;
        std::string top_transition_left_texture;
        std::string top_texture;
        std::string top_transition_right_texture;
        std::string top_right_texture;
        std::string right_transition_top_texture;
        std::string right_texture;
        std::string right_transition_bottom_texture;
        std::string bottom_right_texture;
        std::string bottom_transition_right_texture;
        std::string bottom_texture;
        std::string bottom_transition_left_texture;
        std::string bottom_left_texture;
        std::string left_transition_bottom_texture;
        std::string left_texture;
        std::string left_transition_top_texture;
        std::string background_texture;
    };

    // Textures for frames
    struct FrameTextures
    {
        std::string bottom_texture;
        std::string bottom_left_texture;
        std::string bottom_right_texture;
        std::string left_texture;
        std::string right_texture;
        std::string top_texture;
        std::string top_left_texture;
        std::string top_right_texture;
        std::string background_texture;
    };

    // Textures for buttons
    struct ButtonTextures
    {
        struct State
        {
            std::string left_texture;
            std::string middle_texture;
            std::string right_texture;
        };

        State normal;
        State mouse_over;
        State pressed;
        State disabled;
    };

    // Textures for checkboxes
    struct CheckboxTextures
    {
        std::string off_texture;
        std::string on_texture;
    };

    // Textures for radio buttons
    struct RadioButtonTextures
    {
        std::string off_texture;
        std::string on_texture;
        std::string disabled_texture;
        std::string mouse_over_texture;
    };

    // Textures for track bars
    // A track bar is used to scroll between a range of values
    struct TrackbarTextures
    {
        struct Button
        {
            std::string normal_texture;
            std::string pressed_texture;
            std::string mouse_over_texture;
            std::string disabled_texture;
        };

        struct ScrollArea
        {
            // The 'scroll area'
            std::string start_texture;
            std::string middle_texture;
            std::string end_texture;

            // The scroll tab
            std::string tab_texture;
        };

        ScrollArea normal;
        ScrollArea disabled;

        Button decrease_button;
        Button increase_button;
    };

    // Textures for scroll bars
    // A scroll bar is used to scroll a larger content in a smaller window.
    struct ScrollBarTextures
    {
        struct Button
        {
            std::string normal_texture;
            std::string pressed_texture;
            std::string mouse_over_texture;
            std::string disabled_texture;
        };

        struct ScrollArea
        {
            // The 'scroll area'
            std::string start_texture;
            std::string middle_texture;
            std::string end_texture;

            // The scroll tab
            std::string tab_texture;
        };

        ScrollArea normal;
        ScrollArea disabled;

        Button decrease_button;
        Button increase_button;
    };

    // Textures for comboboxes
    struct ComboBoxTextures
    {
        std::string popdown_button_texture;
        std::string popdown_button_mouse_over_texture;
        std::string popdown_button_pressed_texture;
        std::string text_box_texture;
        std::string left_cap_texture;
    };

    // Textures for progress bars
    struct ProgressBarTextures
    {
        std::string left_texture;
        std::string middle_off_texture;
        std::string middle_on_texture;
        std::string right_texture;
    };

    // Textures for images
    struct ImageTextures
    {
        // Scanline texture that appears on top of the image
        std::string scanlines_texture;
    };

    /**
     * Constructs a UI Style Store and loads all styles from the given XML file.
     *
     * The UI styles can be applied to widgets (dialogs, buttons, etc.) to change their appearance.
     */
    StyleStore(AssetLoader& asset_loader, std::string_view xml_filename);

    /**
     * Returns the filename for the mega texture component.
     *
     * This is the filename specified in the XML as the high-resolution, uncompressed mega texture.
     * All texture names used in styles are stored in this mega texture.
     */
    std::string_view mega_texture_filename() const noexcept
    {
        return m_mega_texture_filename;
    }

    const DialogTextures&      dialog_textures(std::string_view control_name) const noexcept;
    const FrameTextures&       frame_textures(std::string_view control_name) const noexcept;
    const ButtonTextures&      button_textures(std::string_view control_name) const noexcept;
    const CheckboxTextures&    checkbox_textures(std::string_view control_name) const noexcept;
    const RadioButtonTextures& radiobutton_textures(std::string_view control_name) const noexcept;
    const TrackbarTextures&    trackbar_textures(std::string_view control_name,
                                                 bool             is_vertical) const noexcept;
    const ScrollBarTextures&   scrollbar_textures(std::string_view control_name) const noexcept;
    const ComboBoxTextures&    combobox_textures(std::string_view control_name) const noexcept;
    const ProgressBarTextures& progressbar_textures(std::string_view control_name) const noexcept;
    const ImageTextures&       image_textures(std::string_view control_name) const noexcept;

    const FontStyle& button_font(std::string_view control_name) const noexcept;
    const FontStyle& listbox_font(std::string_view control_name) const noexcept;
    const FontStyle& combobox_font(std::string_view control_name) const noexcept;
    const FontStyle& editbox_font(std::string_view control_name) const noexcept;
    const FontStyle& label_font(std::string_view control_name) const noexcept;
    const FontStyle& image_font(std::string_view control_name) const noexcept;

private:
    struct ControlTextures
    {
        DialogTextures      dialog_textures;
        FrameTextures       frame_textures;
        ButtonTextures      button_textures;
        CheckboxTextures    checkbox_textures;
        RadioButtonTextures radiobutton_textures;
        TrackbarTextures    trackbar_horz_textures;
        TrackbarTextures    trackbar_vert_textures;
        ScrollBarTextures   scrollbar_textures;
        ComboBoxTextures    combobox_textures;
        ProgressBarTextures progressbar_textures;
        ImageTextures       image_textures;
    };

    enum class FontControlType
    {
        Button,
        ListBox,
        ComboBox,
        EditBox,
        Label,
        Image,
    };
    static constexpr unsigned int NUM_CONTROL_TYPES = 6;

    using ControlTexturesMap = std::map<std::string, ControlTextures, khepri::CaseInsensitiveLess>;
    using FontStyleMap       = std::map<std::string, FontStyle, khepri::CaseInsensitiveLess>;

    struct TextureData
    {
        ControlTextures    default_textures;
        ControlTexturesMap override_textures;
    };

    struct FontData
    {
        std::array<FontStyle, NUM_CONTROL_TYPES>                      default_fonts;
        std::map<std::string, FontStyle, khepri::CaseInsensitiveLess> override_fonts;
    };

    static TextureData parse_default_textures(const openglyph::XmlParser::Node& default_node);
    static TextureData parse_textures(const openglyph::XmlParser::Node& textures_node);
    static FontData    parse_fonts(const openglyph::XmlParser::Node& fonts_node);

    template <typename TextureType>
    const TextureType&
    control_textures(std::string_view  control_name,
                     const TextureType ControlTextures::* control_textures) const noexcept;

    const FontStyle& control_font(std::string_view control_name,
                                  FontControlType  control_type) const noexcept;

    std::string m_mega_texture_filename;

    TextureData m_textures;
    FontData    m_fonts;
};

} // namespace openglyph::ui