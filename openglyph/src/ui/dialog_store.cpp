#include <khepri/io/exceptions.hpp>
#include <khepri/math/point.hpp>
#include <khepri/math/size.hpp>
#include <khepri/ui/align.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/parser/exceptions.hpp>
#include <openglyph/ui/dialog_store.hpp>

#include <charconv>
#include <variant>

using namespace std::literals;

namespace openglyph::ui {
namespace {

struct IdDefinition
{
    std::string name;
    int         id;
};

std::string read_stream(khepri::io::Stream& stream)
{
    const auto  size = stream.seek(0, khepri::io::SeekOrigin::end);
    std::string buffer(static_cast<std::size_t>(size), '\0');
    stream.seek(0, khepri::io::SeekOrigin::begin);
    if (stream.read(buffer.data(), buffer.size()) != buffer.size()) {
        throw khepri::io::Error("unable to read stream");
    }
    return buffer;
}

template <typename Container, typename T>
bool contains(const Container& container, const T& element) noexcept
{
    return std::find(std::begin(container), std::end(container), element) != std::end(container);
}

auto parse_resource_header(khepri::io::Stream& stream)
{
    const auto data = read_stream(stream);
    // We could implement a preprocessor here, but we only need to extract #define statements.
    // There might be some #defines that are under an #ifdef that we'll parse anyway, but those
    // aren't relevant to us. Crude, but it works.
    std::vector<IdDefinition> definitions;
    khepri::Tokenizer         tokenizer(std::string_view(data.data(), data.size()), " ,\t\r\n");
    while (const auto token = tokenizer.next()) {
        if (khepri::case_insensitive_equals(*token, "#define")) {
            const auto name = tokenizer.next();
            const auto id   = tokenizer.next();
            if (name && id) {
                int value;
                if (auto [ptr, ec] = std::from_chars(id->data(), id->data() + id->size(), value);
                    ec == std::errc{} && ptr == id->data() + id->size()) {
                    definitions.push_back({std::string(*name), value});
                }
            }
        }
    }

    return definitions;
}

class RcLexer final
{
public:
    struct IntegerToken
    {
        int value;
    };

    struct IdentifierToken
    {
        std::string_view name;
    };

    struct StringToken
    {
        std::string_view str;
    };

    struct PunctuatorToken
    {
        char punct;
    };

    using Token = std::variant<IntegerToken, IdentifierToken, StringToken, PunctuatorToken>;

    explicit RcLexer(std::string_view input) : m_input(input)
    {
        advance();
    }

    std::optional<Token> token()
    {
        return token_;
    }

    void advance()
    {
        token_ = next_token();
    }

private:
    std::optional<Token> next_token()
    {
        for (;;) {
            // skip whitespace
            while (m_next < m_input.size() && std::isspace(m_input[m_next])) {
                ++m_next;
            }
            if (m_next == m_input.size()) {
                // Done, no more tokens
                break;
            }

            if (m_next + 1 < m_input.size() && m_input[m_next] == '/' &&
                m_input[m_next + 1] == '/') {
                // Single-line comment
                m_next += 2;
                skip_to_eol();
                continue;
            }

            if (m_next + 1 < m_input.size() && m_input[m_next] == '/' &&
                m_input[m_next + 1] == '*') {
                // Multi-line comment
                m_next += 2;
                while (m_next + 1 < m_input.size() &&
                       !(m_input[m_next] == '*' && m_input[m_next + 1] == '/')) {
                    ++m_next;
                }
                continue;
            }

            if (m_input[m_next] == '#') {
                // Preprocessor directive, ignore the entire line for our purposes as if it were a
                // comment
                skip_to_eol();
                continue;
            }

            if (m_input[m_next] == '"') {
                // String literal
                const auto start = m_next + 1;
                m_next           = m_input.find('"', start);
                if (m_next == std::string_view::npos) {
                    throw openglyph::ParseError("unterminated string in RC file");
                }
                const auto str = m_input.substr(start, m_next - start);
                m_next++;
                return StringToken{str};
            }

            if (m_next + 1 < m_input.size() && m_input[m_next] == '0' &&
                std::toupper(m_input[m_next + 1]) == 'X') {
                // Hex number
                m_next += 2;
                return IntegerToken{parse_number(16)};
            }

            if (std::isdigit(m_input[m_next]) != 0) {
                // Decimal number
                return IntegerToken{parse_number(10)};
            }

            if (std::isalpha(m_input[m_next]) != 0 || m_input[m_next] == '_') {
                // Identifier
                const auto start = m_next;
                while (m_next < m_input.size() &&
                       (std::isalnum(m_input[m_next]) != 0 || m_input[m_next] == '_')) {
                    ++m_next;
                }
                return IdentifierToken{m_input.substr(start, m_next - start)};
            }

            if (m_input[m_next] == '|' || m_input[m_next] == ',') {
                return PunctuatorToken{m_input[m_next++]};
            }

            throw openglyph::ParseError("invalid character in RC file");
        }
        return {};
    }

    void skip_to_eol()
    {
        while (m_next < m_input.size() && m_input[m_next] != '\n') {
            ++m_next;
        }
    }

    int parse_number(int base)
    {
        int value;
        const auto [ptr, ec] =
            std::from_chars(m_input.data() + m_next, m_input.data() + m_input.size(), value, base);
        if (ec != std::errc{}) {
            // invalid number
            throw openglyph::ParseError("invalid number in RC file");
        }
        m_next = static_cast<std::size_t>(ptr - m_input.data());
        return value;
    }

    std::string_view     m_input;
    std::size_t          m_next{0};
    std::optional<Token> token_;
};

class RcParser final
{
public:
    explicit RcParser(khepri::io::Stream& stream, gsl::span<const IdDefinition> id_definitions)
        : m_data(read_stream(stream)), m_lexer(m_data), m_id_definitions(id_definitions)
    {
    }

    std::vector<DialogDefinition> parse()
    {
        std::vector<DialogDefinition> dialogs;
        if (try_identifier("LANGUAGE"sv)) {
            // Ignore language directives
            expect_identifier();
            expect_punctuator(',');
            expect_identifier();
        }

        while (m_lexer.token()) {
            if (try_identifier("GUIDELINES")) {
                // Guidelines always come after the dialogs, so we can stop parsing here.
                break;
            }

            // A name-based resource. The name comes first
            const auto name = expect_identifier();
            if (try_identifier("DIALOG"sv) || try_identifier("DIALOGEX"sv)) {
                dialogs.push_back(parse_dialog(name));
            } else {
                throw openglyph::ParseError("expected DIALOG or DIALOGEX in RC file");
            }
        }
        return dialogs;
    }

private:
    DialogDefinition parse_dialog(std::string_view name)
    {
        DialogDefinition dialog;
        dialog.name = std::string(name);
        expect_integer(); // x
        expect_punctuator(',');
        expect_integer(); // y
        expect_punctuator(',');
        dialog.size.width = expect_integer();
        expect_punctuator(',');
        dialog.size.height = expect_integer();
        // Parse optional statements
        while (!try_identifier("BEGIN"sv)) {
            if (try_identifier("STYLE"sv)) {
                // We don't care about dialog styles
                parse_styles();
            } else if (try_identifier("CAPTION"sv)) {
                dialog.title = expect_string();
            } else if (try_identifier("FONT"sv)) {
                dialog.font_name = parse_font();
            } else {
                throw openglyph::ParseError("unknown optional statement \"" +
                                            std::string(expect_identifier()) +
                                            "\" in DIALOG definition");
            }
        }
        // Parse controls
        while (!try_identifier("END"sv)) {
            if (auto control = parse_control()) {
                // Resolve the control's ID from its name
                std::visit(
                    [&](auto& ctrl) {
                        if (const auto it = std::find_if(
                                m_id_definitions.begin(), m_id_definitions.end(),
                                [&](const IdDefinition& def) { return def.name == ctrl.name; });
                            it != m_id_definitions.end()) {
                            ctrl.id = it->id;
                        }
                    },
                    *control);

                dialog.controls.push_back(std::move(*control));
            }
        }
        return dialog;
    }

    std::optional<ControlDefinition> parse_control()
    {
        if (try_identifier("PUSHBUTTON"sv, "DEFPUSHBUTTON"sv)) {
            return parse_button_control();
        } else if (const auto control_type = try_identifier("LTEXT"sv, "CTEXT"sv, "RTEXT"sv)) {
            return parse_label_control(*control_type);
        } else if (try_identifier("EDITTEXT"sv)) {
            return parse_edit_box_control();
        } else if (try_identifier("COMBOBOX"sv)) {
            return parse_combo_box_control();
        } else if (try_identifier("LISTBOX"sv)) {
            return parse_list_box_control();
        } else if (try_identifier("SCROLLBAR"sv)) {
            return parse_scroll_bar_control();
        } else if (try_identifier("GROUPBOX"sv)) {
            return parse_frame_control();
        } else if (try_identifier("CONTROL"sv)) {
            return parse_generic_control();
        } else {
            throw openglyph::ParseError("unknown control in DIALOG definition");
        }
    }

    PushButtonControlDefinition parse_button_control()
    {
        PushButtonControlDefinition button;
        button.text = expect_string();
        expect_punctuator(',');
        button.name = expect_identifier();
        expect_punctuator(',');
        parse_control_dimensions(button);
        if (try_punctuator(',')) {
            // Parse optional styles
            for (const auto style : parse_styles()) {
                if (khepri::case_insensitive_equals(style, "BS_LEFT"sv)) {
                    button.text_alignment = khepri::ui::TextAlign::left;
                } else if (khepri::case_insensitive_equals(style, "BS_CENTER"sv)) {
                    button.text_alignment = khepri::ui::TextAlign::center;
                } else if (khepri::case_insensitive_equals(style, "BS_RIGHT"sv)) {
                    button.text_alignment = khepri::ui::TextAlign::right;
                }
            }
        }
        return button;
    }

    LabelControlDefinition parse_label_control(std::string_view control_type)
    {
        LabelControlDefinition label;
        label.text           = expect_string();
        label.text_alignment = [&] {
            if (control_type == "CTEXT"sv) {
                return khepri::ui::TextAlign::center;
            } else if (control_type == "RTEXT"sv) {
                return khepri::ui::TextAlign::right;
            } else {
                return khepri::ui::TextAlign::left;
            }
        }();
        expect_punctuator(',');
        label.name = expect_identifier();
        expect_punctuator(',');
        parse_control_dimensions(label);
        return label;
    }

    EditBoxControlDefinition parse_edit_box_control()
    {
        EditBoxControlDefinition edit;
        edit.name = expect_identifier();
        expect_punctuator(',');
        parse_control_dimensions(edit);
        expect_punctuator(',');
        for (const auto style : parse_styles()) {
            if (khepri::case_insensitive_equals(style, "ES_CENTER"sv)) {
                edit.text_alignment = khepri::ui::TextAlign::center;
            } else if (khepri::case_insensitive_equals(style, "ES_RIGHT"sv)) {
                edit.text_alignment = khepri::ui::TextAlign::right;
            }
        }
        return edit;
    }

    ComboBoxControlDefinition parse_combo_box_control()
    {
        ComboBoxControlDefinition combo;
        combo.name = expect_identifier();
        expect_punctuator(',');
        parse_control_dimensions(combo);
        expect_punctuator(',');
        parse_styles(); // ignore styles
        return combo;
    }

    ListBoxControlDefinition parse_list_box_control()
    {
        ListBoxControlDefinition listbox;
        listbox.name = expect_identifier();
        expect_punctuator(',');
        parse_control_dimensions(listbox);
        expect_punctuator(',');
        parse_styles(); // ignore styles
        return listbox;
    }

    ScrollBarControlDefinition parse_scroll_bar_control()
    {
        ScrollBarControlDefinition scrollbar;
        scrollbar.name          = expect_identifier();
        scrollbar.is_horizontal = true;
        expect_punctuator(',');
        parse_control_dimensions(scrollbar);
        expect_punctuator(',');
        for (const auto style : parse_styles()) {
            if (khepri::case_insensitive_equals(style, "SBS_VERT"sv)) {
                scrollbar.is_horizontal = false;
            }
        }
        return scrollbar;
    }

    FrameControlDefinition parse_frame_control()
    {
        FrameControlDefinition frame;
        expect_string(); // ignore text
        expect_punctuator(',');
        frame.name = expect_identifier();
        expect_punctuator(',');
        parse_control_dimensions(frame);
        return frame;
    }

    std::optional<ControlDefinition> parse_generic_control()
    {
        const auto text = std::string(expect_string());
        expect_punctuator(',');
        const auto name = std::string(expect_identifier());
        expect_punctuator(',');
        const auto type = expect_string();
        expect_punctuator(',');
        const auto styles = parse_styles();
        expect_punctuator(',');

        auto control = [&]() -> std::optional<ControlDefinition> {
            if (khepri::case_insensitive_equals(type, "Button"sv)) {
                if (contains(styles, "BS_AUTOCHECKBOX"sv)) {
                    CheckBoxControlDefinition checkbox;
                    checkbox.text = std::move(text);
                    return checkbox;
                } else if (contains(styles, "BS_AUTORADIOBUTTON"sv)) {
                    RadioButtonControlDefinition radio;
                    radio.text         = std::move(text);
                    radio.is_push_like = contains(styles, "BS_PUSHLIKE"sv);
                    return radio;
                }
            } else if (khepri::case_insensitive_equals(type, "msctls_trackbar32"sv)) {
                TrackBarControlDefinition trackbar;
                trackbar.name          = std::move(name);
                trackbar.is_horizontal = !contains(styles, "TBS_VERT"sv);
                return trackbar;

            } else if (khepri::case_insensitive_equals(type, "msctls_progress32"sv)) {
                ProgressBarControlDefinition progressbar;
                return progressbar;
            } else if (khepri::case_insensitive_equals(type, "SysListView32"sv)) {
                ListBoxControlDefinition listbox;
                return listbox;
            } else if (khepri::case_insensitive_equals(type, "PETROGLYPH_DIALOG_IMAGE"sv)) {
                ImageControlDefinition image;
                image.image_name = std::move(text);
                return image;
            } else if (khepri::case_insensitive_equals(type, "IMEEditBox"sv) ||
                       khepri::case_insensitive_equals(type, "IMELocaleIndicator"sv)) {
                // IME control types can occur, but only in unused 'test' dialogs. The game doesn't
                // support them, so we ignore them.
                // Still need to parse the dimensions though.
                CommonControlDefinition dummy_ctrl;
                parse_control_dimensions(dummy_ctrl);
                return {};
            }
            throw openglyph::ParseError("unknown CONTROL type \"" + std::string(type) +
                                        "\" in RC file");
        }();

        if (control) {
            std::visit(
                [&](auto& ctrl) {
                    ctrl.name = std::move(name);
                    parse_control_dimensions(ctrl);
                },
                *control);
        }

        return control;
    }

    void parse_control_dimensions(CommonControlDefinition& control)
    {
        control.position.x = expect_integer();
        expect_punctuator(',');
        control.position.y = expect_integer();
        expect_punctuator(',');
        control.size.width = expect_integer();
        expect_punctuator(',');
        control.size.height = expect_integer();
    }

    // Parse |-separated styles
    std::vector<std::string_view> parse_styles()
    {
        // Note: we only support named styles
        std::vector<std::string_view> styles;
        do {
            styles.push_back(expect_identifier());
        } while (try_punctuator('|'));
        return styles;
    }

    // Parse font, return the font name (the only part we care about)
    std::string_view parse_font()
    {
        expect_integer(); // size
        expect_punctuator(',');
        auto font_name = expect_string();
        expect_punctuator(',');
        expect_integer(); // weight
        expect_punctuator(',');
        expect_integer(); // italic
        expect_punctuator(',');
        expect_integer(); // charset
        return font_name;
    }

    std::string_view expect_identifier()
    {
        if (auto ident = try_identifier()) {
            return *ident;
        }
        throw openglyph::ParseError("expected identifier in RC file");
    }

    template <typename... Args,
              std::enable_if_t<(std::conjunction_v<std::is_convertible<Args, std::string_view>...>),
                               int> = 0>
    std::optional<std::string_view> try_identifier(const Args&... names)
    {
        if (auto token = m_lexer.token();
            token && std::holds_alternative<RcLexer::IdentifierToken>(*token)) {
            const auto ident = std::get<RcLexer::IdentifierToken>(*token).name;
            for (const auto& name : {names...}) {
                if (khepri::case_insensitive_equals(ident, name)) {
                    m_lexer.advance();
                    return name;
                }
            }
        }
        return {};
    }

    std::optional<std::string_view> try_identifier()
    {
        if (auto token = m_lexer.token();
            token && std::holds_alternative<RcLexer::IdentifierToken>(*token)) {
            m_lexer.advance();
            return std::get<RcLexer::IdentifierToken>(*token).name;
        }
        return {};
    }

    int expect_integer()
    {
        if (auto value = try_integer()) {
            return *value;
        }
        throw openglyph::ParseError("expected integer in RC file");
    }

    std::optional<int> try_integer()
    {
        if (auto token = m_lexer.token();
            token && std::holds_alternative<RcLexer::IntegerToken>(*token)) {
            m_lexer.advance();
            return std::get<RcLexer::IntegerToken>(*token).value;
        }
        return {};
    }

    void expect_punctuator(char punct)
    {
        if (!try_punctuator(punct)) {
            throw openglyph::ParseError("expected punctuator in RC file");
        }
    }

    std::string_view expect_string()
    {
        if (auto token = m_lexer.token();
            token && std::holds_alternative<RcLexer::StringToken>(*token)) {
            m_lexer.advance();
            return std::get<RcLexer::StringToken>(*token).str;
        }
        throw openglyph::ParseError("expected string in RC file");
    }

    bool try_punctuator(char punct)
    {
        if (auto token = m_lexer.token();
            token && std::holds_alternative<RcLexer::PunctuatorToken>(*token)) {
            if (std::get<RcLexer::PunctuatorToken>(*token).punct == punct) {
                m_lexer.advance();
                return true;
            }
        }
        return false;
    }

    std::string                   m_data;
    RcLexer                       m_lexer;
    gsl::span<const IdDefinition> m_id_definitions;
};

} // namespace

DialogStore::DialogStore(AssetLoader& asset_loader, std::string_view rc_filename,
                         std::string_view h_filename)
{
    // First parse the .h file to extract the preprocessor definitions that are used in the .rc file
    const auto h_file  = asset_loader.open_resource(h_filename);
    const auto rc_file = asset_loader.open_resource(rc_filename);
    if (!h_file || !rc_file) {
        // TODO: exception type
        throw std::runtime_error("unable to open dialog file");
    }

    const auto id_definitions = parse_resource_header(*h_file);
    RcParser   parser(*rc_file, id_definitions);
    for (auto& dialog : parser.parse()) {
        m_dialogs.emplace(dialog.name, std::move(dialog));
    }
}

const DialogDefinition* DialogStore::find_dialog(std::string_view name) const noexcept
{
    if (const auto it = m_dialogs.find(name); it != m_dialogs.end()) {
        return &it->second;
    }
    return nullptr;
}

} // namespace openglyph::ui