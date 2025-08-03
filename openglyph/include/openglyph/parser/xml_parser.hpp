#pragma once
#include "exceptions.hpp"

#include <khepri/io/stream.hpp>

#include <optional>
#include <rapidxml.hpp>
#include <string_view>
#include <vector>

namespace openglyph {

/**
 * @brief A parser for XML content
 *
 * The parser present a DOM-like view on the parsed document, allowing one to iterate over all of
 * the document's nodes and its attributes.
 */
class XmlParser
{
    using Char = char;

public:
    /**
     * An attribute on an XML node.
     */
    class Attribute
    {
    public:
        explicit Attribute(const rapidxml::xml_attribute<Char>* attr) : m_attr(attr) {}

        /// The attribute's name
        [[nodiscard]] std::string_view name() const noexcept
        {
            return {m_attr->name(), m_attr->name_size()};
        }

        /// The attribute's value
        [[nodiscard]] std::string_view value() const noexcept
        {
            return {m_attr->value(), m_attr->value_size()};
        }

    private:
        const rapidxml::xml_attribute<Char>* m_attr;
    };

    /**
     * An iterable range of attributes on an XML node.
     */
    class AttributeRange
    {
    public:
        class Iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type        = Attribute;
            using difference_type   = std::ptrdiff_t;
            using pointer           = Attribute*;
            using reference         = Attribute&;

            explicit Iterator(const rapidxml::xml_attribute<Char>* cur) noexcept : m_cur(cur) {}

            Iterator& operator++() noexcept
            {
                m_cur = m_cur->next_attribute();
                return *this;
            }

            Iterator operator++(int) & noexcept
            {
                Iterator retval = *this;
                ++(*this);
                return retval;
            }

            bool operator==(Iterator other) const noexcept
            {
                return m_cur == other.m_cur;
            }

            bool operator!=(Iterator other) const noexcept
            {
                return !(*this == other);
            }

            Attribute operator*() const noexcept
            {
                return Attribute(m_cur);
            }

        private:
            const rapidxml::xml_attribute<Char>* m_cur;
        };

        explicit AttributeRange(const rapidxml::xml_attribute<Char>* first) noexcept
            : m_first(first)
        {
        }

        [[nodiscard]] Iterator begin() const noexcept
        {
            return Iterator(m_first);
        }

        [[nodiscard]] Iterator end() const noexcept
        {
            return Iterator(nullptr);
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return m_first != nullptr;
        }

    private:
        const rapidxml::xml_attribute<Char>* m_first;
    };

    class Node;

    /**
     * An iterable range of XML nodes, typically child nodes of a single parent node.
     */
    class NodeRange
    {
    public:
        class Iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type        = Node;
            using difference_type   = std::ptrdiff_t;
            using pointer           = Node*;
            using reference         = Node&;

            explicit Iterator(const rapidxml::xml_node<Char>* cur) noexcept : m_cur(cur) {}

            Iterator& operator++() noexcept
            {
                m_cur = m_cur->next_sibling();
                return *this;
            }

            Iterator operator++(int) & noexcept
            {
                Iterator retval = *this;
                ++(*this);
                return retval;
            }

            bool operator==(Iterator other) const noexcept
            {
                return m_cur == other.m_cur;
            }

            bool operator!=(Iterator other) const noexcept
            {
                return !(*this == other);
            }

            Node operator*() const noexcept
            {
                return Node(m_cur);
            }

        private:
            const rapidxml::xml_node<Char>* m_cur;
        };

        explicit NodeRange(const rapidxml::xml_node<Char>* first) noexcept : m_first(first) {}

        [[nodiscard]] Iterator begin() const noexcept
        {
            return Iterator(m_first);
        }

        [[nodiscard]] Iterator end() const noexcept
        {
            return Iterator(nullptr);
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return m_first != nullptr;
        }

    private:
        const rapidxml::xml_node<Char>* m_first;
    };

    /**
     * An XML node.
     *
     * A node is the entity in an XML document making up the DOM tree.
     */
    class Node
    {
    public:
        explicit Node(const rapidxml::xml_node<Char>* node) : m_node(node)
        {
            assert(m_node != nullptr);
        }

        /// The node's name
        [[nodiscard]] std::string_view name() const noexcept
        {
            return {m_node->name(), m_node->name_size()};
        }

        /// The node's text content
        [[nodiscard]] std::string_view value() const noexcept
        {
            return {m_node->value(), m_node->value_size()};
        }

        /// The node's attributes
        [[nodiscard]] AttributeRange attributes() const noexcept
        {
            return AttributeRange(m_node->first_attribute());
        }

        /// The node's child nodes
        [[nodiscard]] NodeRange nodes() const noexcept
        {
            return NodeRange(m_node->first_node());
        }

        /**
         * @brief Find an attribute by name
         *
         * Returns the value of the node's attribute that has the specified name. If multiple
         * attributes match, returns the first. If no attributes match, returns @a std::nullopt.
         *
         * @note the name lookup is case-insensitive.
         */
        [[nodiscard]] std::optional<std::string_view>
        attribute(std::string_view name) const noexcept
        {
            const auto* attr = m_node->first_attribute(name.data(), name.size(), false);
            if (attr != nullptr) {
                return std::string_view{attr->value(), attr->value_size()};
            }
            return {};
        }

        /**
         * @brief Find a child node by name
         *
         * Returns the node's child node that has the specified name. If multiple
         * child nodes match, returns the first. If no child nodes match, returns @a std::nullopt.
         *
         * @note the name lookup is case-insensitive.
         */
        [[nodiscard]] std::optional<Node> child(std::string_view name) const noexcept
        {
            const auto* xml_node = m_node->first_node(name.data(), name.size(), false);
            if (xml_node != nullptr) {
                return Node(xml_node);
            }
            return {};
        }

    private:
        const rapidxml::xml_node<Char>* m_node;
    };

    /**
     * Reads and parses a stream containing XML data.
     *
     * After creation, use @ref root() to obtain the root node of the document and iterate over the
     * rest of the document.
     *
     * @throws khepri::io::Error if an I/O error occured during reading
     * @throws openglyph::ParseError if an error occured during parsing
     */
    explicit XmlParser(khepri::io::Stream& stream);

    /**
     * @brief Returns the root node of the XML document, if any.
     */
    [[nodiscard]] std::optional<Node> root() const noexcept;

private:
    std::vector<Char> m_data;

    // Note: RapidXML holds a reference to the string contents
    rapidxml::xml_document<Char> m_document;
};

/**
 * Returns the value of a required node attribute.
 *
 * Returns the value of the node's attribute that has the specified name. If multiple
 * attributes match, returns the first.
 *
 * @note the name lookup is case-insensitive.
 *
 * @throws openglyph::ParseError if the node has no such attribute.
 */
inline std::string_view require_attribute(const openglyph::XmlParser::Node& node,
                                          std::string_view                  name)
{
    if (auto value = node.attribute(name)) {
        return *value;
    }
    throw ParseError("missing attribute \"" + std::string(name) + "\"");
}

/**
 * Returns the value of an optional node attribute with default.
 *
 * Returns the value of the node's attribute that has the specified name. If multiple
 * attributes match, returns the first. If no attributes match, returns @a default_value.
 *
 * @note the name lookup is case-insensitive.
 */
inline std::string_view optional_attribute(const openglyph::XmlParser::Node& node,
                                           std::string_view name, std::string_view default_value)
{
    if (auto value = node.attribute(name)) {
        return *value;
    }
    return default_value;
}

/**
 * Returns the contents of a required child node.
 *
 * Returns the contents of the node's child that has the specified name. If multiple
 * children match, returns the first.
 *
 * @note the name lookup is case-insensitive.
 *
 * @throws openglyph::ParseError if the node has no such child, or if the child has children of its
 *                               own.
 */
inline std::string_view require_child(const openglyph::XmlParser::Node& node, std::string_view name,
                                      std::string_view default_value)
{
    if (auto child = node.child(name)) {
        if (child->nodes().empty()) {
            return child->value();
        }
    }
    throw ParseError("missing child node \"" + std::string(name) + "\" with text content");
}

/**
 * Returns the contents of an optional child node or std::nullopt if it's missing.
 *
 * Returns the contents of the node's child that has the specified name. If multiple
 * children match, returns the first. If no children match, or if the child has children of its own,
 * returns @a std::nullopt.
 *
 * @note the name lookup is case-insensitive.
 */
inline std::optional<std::string_view> optional_child(const openglyph::XmlParser::Node& node,
                                                      std::string_view                  name)
{
    if (auto child = node.child(name)) {
        if (child->nodes().empty()) {
            return child->value();
        }
    }
    return {};
}

/**
 * Returns the contents of an optional child node, or a default value if its missing.
 *
 * Returns the contents of the node's child that has the specified name. If multiple
 * children match, returns the first. If no children match, or if the child has children of its own,
 * returns @a default_value.
 *
 * @note the name lookup is case-insensitive.
 */
inline std::string_view optional_child(const openglyph::XmlParser::Node& node,
                                       std::string_view name, std::string_view default_value)
{
    if (auto child = node.child(name)) {
        if (child->nodes().empty()) {
            return child->value();
        }
    }
    return default_value;
}

} // namespace openglyph
