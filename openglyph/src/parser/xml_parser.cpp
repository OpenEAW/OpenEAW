#include <openglyph/parser/xml_parser.hpp>

#include <algorithm>

namespace openglyph {

XmlParser::XmlParser(khepri::io::Stream& stream)
    : m_data(static_cast<std::size_t>(stream.seek(0, khepri::io::SeekOrigin::end)) / sizeof(Char) +
             1)
{
    stream.seek(0, khepri::io::SeekOrigin::begin);
    stream.read(m_data.data(), (m_data.size() - 1) * sizeof(Char));
    m_data.back() = '\0';
    try {
        // This modifies and holds references to the input string.
        // parse_no_string_terminators saves some performance by not adding \0 by relying on a
        // passed size.
        m_document.parse<rapidxml::parse_no_string_terminators>(m_data.data());
    } catch (const rapidxml::parse_error& e) {
        std::size_t       line  = 0;
        const auto* const where = e.where<Char>();
        if (where >= &m_data.front() && where <= &m_data.back()) {
            line = std::count<const char*>(m_data.data(), where, '\n') + 1;
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
