#include <khepri/io/exceptions.hpp>
#include <khepri/io/stream.hpp>

#include <cassert>
#include <cstdint>
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

int Stream::read_short()
{
    std::int16_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

long Stream::read_int()
{
    std::int32_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

unsigned char Stream::read_byte()
{
    std::int8_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

unsigned int Stream::read_ushort()
{
    std::int16_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

unsigned long Stream::read_uint()
{
    std::int32_t x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

float Stream::read_float()
{
    float x{};
    read_checked(*this, &x, sizeof x);
    return x;
}

std::string Stream::read_string()
{
    std::vector<char> x(read_ushort());
    if (!x.empty()) {
        read_checked(*this, x.data(), x.size());
        return {x.begin(), x.end()};
    }
    return "";
}

void Stream::write_short(int s)
{
    const auto x = static_cast<std::uint16_t>(static_cast<unsigned int>(s));
    write_checked(*this, &x, sizeof x);
}

void Stream::write_int(long i)
{
    const auto x = static_cast<std::uint32_t>(static_cast<unsigned long>(i));
    write_checked(*this, &x, sizeof x);
}

void Stream::write_byte(unsigned char b)
{
    const auto x = static_cast<std::uint8_t>(b);
    write_checked(*this, &x, sizeof x);
}

void Stream::write_ushort(unsigned int s)
{
    const auto x = static_cast<std::uint16_t>(s);
    write_checked(*this, &x, sizeof x);
}

void Stream::write_uint(unsigned long i)
{
    const auto x = static_cast<std::uint32_t>(i);
    write_checked(*this, &x, sizeof x);
}

void Stream::write_float(float f)
{
    write_checked(*this, &f, sizeof f);
}

void Stream::write_string(const std::string& s)
{
    assert(s.size() <= std::numeric_limits<uint16_t>::max());
    write_ushort(static_cast<unsigned short>(s.size()));
    write_checked(*this, s.c_str(), s.size());
}

} // namespace khepri::io
