#include <khepri/io/exceptions.hpp>
#include <khepri/io/stream.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace khepri::io {

namespace {
void read_checked(Stream& stream, void* data, std::size_t count)
{
    if (stream.read(data, count) != count) {
        throw Error("Unable to read from stream");
    }
}

void write_checked(Stream& stream, const void* data, std::size_t count)
{
    if (stream.write(data, count) != count) {
        throw Error("Unable to write to stream");
    }
}
} // namespace

std::int16_t Stream::read_int16()
{
    std::int16_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

std::int32_t Stream::read_int32()
{
    std::int32_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

std::int64_t Stream::read_int64()
{
    std::int64_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

float Stream::read_float()
{
    float x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

std::uint8_t Stream::read_uint8()
{
    std::int8_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

std::uint16_t Stream::read_uint16()
{
    std::int16_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

std::uint32_t Stream::read_uint32()
{
    std::int32_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

std::uint64_t Stream::read_uint64()
{
    std::int64_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

std::string Stream::read_string()
{
    std::vector<char> x(read_uint16());
    if (!x.empty()) {
        read_checked(*this, x.data(), x.size());
        return {x.begin(), x.end()};
    }
    return "";
}

void Stream::write_int8(std::int8_t i8)
{
    write_checked(*this, &i8, sizeof i8);
}

void Stream::write_int16(std::int16_t i16)
{
    write_checked(*this, &i16, sizeof i16);
}

void Stream::write_int32(std::int32_t i32)
{
    write_checked(*this, &i32, sizeof i32);
}

void Stream::write_int64(std::int64_t i64)
{
    write_checked(*this, &i64, sizeof i64);
}

void Stream::write_uint8(std::uint8_t u8)
{
    write_checked(*this, &u8, sizeof u8);
}

void Stream::write_uint16(std::uint16_t u16)
{
    write_checked(*this, &u16, sizeof u16);
}

void Stream::write_uint32(std::uint32_t u32)
{
    write_checked(*this, &u32, sizeof u32);
}

void Stream::write_uint64(std::uint64_t u64)
{
    write_checked(*this, &u64, sizeof u64);
}

void Stream::write_float(float f)
{
    write_checked(*this, &f, sizeof f);
}

void Stream::write_string(const std::string& s)
{
    assert(s.size() <= std::numeric_limits<uint16_t>::max());
    write_uint16(static_cast<unsigned short>(s.size()));
    write_checked(*this, s.c_str(), s.size());
}

} // namespace khepri::io
