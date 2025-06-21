#pragma once

#include <khepri/io/exceptions.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>
#include <khepri/math/vector4.hpp>

#include <gsl/gsl-lite.hpp>

#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

namespace khepri::io {

/**
 * \brief Utility class for serializing data to a binary blob.
 *
 * A serializer contains a buffer where it appends all serialized data.
 * After serializing all data, the buffer can be accessed via #data().
 */
class Serializer final
{
public:
    Serializer() = default;

    /// Constructs a serializer and reserves \a size_hint of storage
    explicit Serializer(std::size_t size_hint)
    {
        m_data.reserve(size_hint);
    }

    /// Serializes \a value and stores it in the serializer's buffer
    template <typename T>
    void write(const T& value);

    /// Serializes byte \a x and stores it in the serializer's buffer
    template <typename T>
    void write(std::uint8_t x)
    {
        m_data.push_back(x);
    }

    /// Returns the buffer with serialized data
    [[nodiscard]] gsl::span<const std::uint8_t> data() const noexcept
    {
        return m_data;
    }

private:
    std::vector<std::uint8_t> m_data;
};

/**
 * \brief Utility class for deserializing data from a binary blob
 *
 * A deserializer contains a buffer where it deserializes values from.
 * Reading elements past the end-of-buffer throws a #khepri::io::Error.
 */
class Deserializer final
{
public:
    /**
     * \brief Constructs a deserializer from a buffer with data.
     * \note This object does NOT take ownership of the data.
     */
    explicit Deserializer(gsl::span<const std::uint8_t> data) noexcept : m_data(data) {}

    /// Deserializes an object from the buffer
    template <typename T>
    T read();

    /// Deserializes a byte from the buffer
    std::uint8_t read_byte()
    {
        if (m_position >= m_data.size()) {
            throw khepri::io::Error("unexpected end of data");
        }
        return m_data[m_position++];
    }

private:
    gsl::span<const std::uint8_t> m_data;
    std::size_t                   m_position{0};
};

namespace detail {
// Helper for serialize_traits for fixed-width unsigned integer types
template <typename T>
struct FixedWidthUintSerializeTraits
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_unsigned_v<T>);

    static constexpr void serialize(Serializer& s, T value)
    {
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            s.write<std::uint8_t>(value & MASK_BYTE);
            value >>= BITS_PER_BYTE;
        }
    }

    static constexpr T deserialize(Deserializer& d)
    {
        T result = 0;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            result |= (static_cast<T>(d.read_byte()) << (i * BITS_PER_BYTE));
        }
        return result;
    }

private:
    static constexpr auto MASK_BYTE     = 0xff;
    static constexpr auto BITS_PER_BYTE = 8;
};

// Helper for serialize_traits for fixed-width signed integer types
template <typename T>
struct FixedWidthIntSerializeTraits
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    static constexpr void serialize(Serializer& s, T value)
    {
        s.write(std::make_unsigned_t<T>(value));
    }

    static constexpr T deserialize(Deserializer& d)
    {
        return static_cast<T>(d.read<std::make_unsigned_t<T>>());
    }
};
} // namespace detail

/**
 * \brief Generic traits class for type-specific (de)serialization code
 *
 * This type is the main customization point for adding (de)serialization support for custom types.
 * Specialize this type with the type you wish to (de)serialize and implement the #serialize and
 * #deserialize methods.
 */
template <typename T>
struct SerializeTraits
{
    /**
     * \brief serializes \a value to serializer \a s
     *
     * Specializations of this method should call \c write in #khepri::io::Serializer with the
     * proper data to serialize \a value.
     */
    static void serialize(Serializer& s, const T& value) = delete;

    /**
     * \brief deserializes a value from deserializer \a s
     *
     * Specializations of this method should call \c read in #khepri::io::Deserializer for the
     * proper types to deserialize data and construct a value of the specialized type.
     */
    static T deserialize(Deserializer& d) = delete;
};

template <typename T>
void Serializer::write(const T& value)
{
    SerializeTraits<T>::serialize(*this, value);
}

template <typename T>
T Deserializer::read()
{
    return SerializeTraits<T>::deserialize(*this);
}

template <>
struct SerializeTraits<std::uint8_t> : detail::FixedWidthUintSerializeTraits<std::uint8_t>
{};

template <>
struct SerializeTraits<std::int8_t> : detail::FixedWidthIntSerializeTraits<std::int8_t>
{};

template <>
struct SerializeTraits<std::uint16_t> : detail::FixedWidthUintSerializeTraits<std::uint16_t>
{};

template <>
struct SerializeTraits<std::int16_t> : detail::FixedWidthIntSerializeTraits<std::int16_t>
{};

template <>
struct SerializeTraits<std::uint32_t> : detail::FixedWidthUintSerializeTraits<std::uint32_t>
{};

template <>
struct SerializeTraits<std::int32_t> : detail::FixedWidthIntSerializeTraits<std::int32_t>
{};

/// Specialization of #khepri::io::SerializeTraits for \c float
template <>
struct SerializeTraits<float>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, float value)
    {
        // NOLINTNEXTLINE we have to reinterpret_cast to get from float to a uint32_t
        s.write(*reinterpret_cast<const std::uint32_t*>(&value));
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static float deserialize(Deserializer& d)
    {
        const auto value = d.read<std::uint32_t>();
        // NOLINTNEXTLINE we have to reinterpret_cast to get from uint32_t to a float
        return *reinterpret_cast<const float*>(&value);
    }
};

/// Specialization of #khepri::io::SerializeTraits for \c gsl::span
template <typename T>
struct SerializeTraits<gsl::span<T>>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const gsl::span<T>& value)
    {
        s.write(static_cast<std::uint32_t>(value.size()));
        for (const auto& elem : value) {
            s.write(elem);
        }
    }

    // Note: gsl::span can't be deserialized since it doesn't own its contents
};

/// Specialization of #khepri::io::SerializeTraits for \c std::vector
template <typename T>
struct SerializeTraits<std::vector<T>>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const std::vector<T>& value)
    {
        s.write(static_cast<std::uint32_t>(value.size()));
        for (const auto& elem : value) {
            s.write(elem);
        }
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static std::vector<T> deserialize(Deserializer& d)
    {
        std::vector<T> data(d.read<std::uint32_t>());
        for (auto& elem : data) {
            elem = d.read<std::decay_t<T>>();
        }
        return data;
    }
};

} // namespace khepri::io
