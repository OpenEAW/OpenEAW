#include <openglyph/parser/xml_parser.hpp>

namespace openglyph {

XmlParser::XmlParser(khepri::io::Stream& stream)
    : m_size(static_cast<std::size_t>(stream.seek(0, khepri::io::SeekOrigin::end)) / sizeof(Char))
    , m_data(std::make_unique<Char[]>(m_size + 1))
{
    auto* data = m_data.get();
    stream.seek(0, khepri::io::SeekOrigin::begin);
    stream.read(data, m_size * sizeof(Char));
    data[m_size] = '\0';
    try {
        // This modifies and holds references to the input string.
        // parse_no_string_terminators saves some performance by not adding \0 by relying on a
        // passed size.
        m_document.parse<rapidxml::parse_no_string_terminators>(data);
    } catch (const rapidxml::parse_error& e) {
        std::size_t line  = 0;
        auto        where = e.where<Char>();
        if (where >= data && where <= data + m_size) {
            line = std::count(data, where, '\n') + 1;
        }
        throw ParseError("XML parse error at line " + std::to_string(line) + ": " + e.what());
    }
}

std::optional<XmlParser::Node> XmlParser::root() const noexcept
{
    const auto* xml_node = m_document.first_node();
    if (xml_node != nullptr) {
        return Node(xml_node);
    }
    return {};
}

} // namespace openglyph
