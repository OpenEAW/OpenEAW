#include <khepri/io/exceptions.hpp>
#include <khepri/io/serialize.hpp>
#include <openglyph/assets/io/map.hpp>
#include <openglyph/io/chunk_reader.hpp>

namespace openglyph::io {
namespace {
enum class ChunkId
{
    map_info = 0x00,
    map_data = 0x01,

    map_data_environment_set    = 0x100,
    map_data_environments       = 0x04,
    map_data_environment        = 0x06,
    map_data_active_environment = 0x08,
};

// Only supported map version
constexpr std::uint32_t MAP_FORMAT_VERSION = 0x201;

void verify(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}

std::string as_string(gsl::span<const uint8_t> data)
{
    auto end = std::find(data.begin(), data.end(), 0);
    return {data.begin(), end};
}

float as_float(gsl::span<const uint8_t> data)
{
    verify(data.size() == sizeof(float));
    return khepri::io::Deserializer(data).read<float>();
}

std::uint32_t as_uint32(gsl::span<const uint8_t> data)
{
    verify(data.size() == sizeof(std::uint32_t));
    return khepri::io::Deserializer(data).read<std::uint32_t>();
}

Map::Header read_map_header(gsl::span<const std::uint8_t> data)
{
    Map::Header     header;
    MinichunkReader reader(data);
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case 0:
            header.version = as_uint32(reader.read_data());
            break;
        }
    }
    return header;
}

auto read_map_environment(gsl::span<const std::uint8_t> data)
{
    Environment     environment;
    MinichunkReader reader(data);
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case 20:
            environment.name = as_string(reader.read_data());
            break;
        case 25:
            environment.skydomes[0].name = as_string(reader.read_data());
            break;
        case 26:
            environment.skydomes[1].name = as_string(reader.read_data());
            break;
        case 27:
            environment.skydomes[0].scale = as_float(reader.read_data());
            break;
        case 28:
            environment.skydomes[1].scale = as_float(reader.read_data());
            break;
        case 29:
            environment.skydomes[0].tilt = khepri::to_radians(as_float(reader.read_data()));
            break;
        case 30:
            environment.skydomes[1].tilt = khepri::to_radians(as_float(reader.read_data()));
            break;
        case 31:
            environment.skydomes[0].z_angle = khepri::to_radians(as_float(reader.read_data()));
            break;
        case 32:
            environment.skydomes[1].z_angle = khepri::to_radians(as_float(reader.read_data()));
            break;
        }
    }
    return environment;
}

auto read_active_environment(gsl::span<const std::uint8_t> data)
{
    std::uint32_t   active_environment = 0;
    MinichunkReader reader(data);
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case 37:
            active_environment = as_uint32(reader.read_data());
            break;
        }
    }
    return active_environment;
}

auto read_map_environments(ChunkReader& reader)
{
    std::vector<Environment> environments;
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case ChunkId::map_data_environment:
            verify(reader.has_data());
            environments.push_back(read_map_environment(reader.read_data()));
            break;
        }
    }
    return environments;
}

auto read_map_environment_set(Map& map, ChunkReader& reader)
{
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case ChunkId::map_data_environments:
            verify(!reader.has_data());
            reader.open();
            map.environments = read_map_environments(reader);
            reader.close();
            break;
        case ChunkId::map_data_active_environment:
            verify(reader.has_data());
            map.active_environment = read_active_environment(reader.read_data());
            break;
        }
    }

    if (map.active_environment >= map.environments.size()) {
        map.active_environment = 0;
    }
}

void read_map_data(Map& map, ChunkReader& reader)
{
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case ChunkId::map_data_environment_set:
            verify(!reader.has_data());
            reader.open();
            read_map_environment_set(map, reader);
            reader.close();
            break;
        }
    }
}

} // namespace

openglyph::Map read_map(khepri::io::Stream& stream)
{
    Map         map;
    ChunkReader reader(stream);
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case ChunkId::map_info:
            verify(reader.has_data());
            map.header = read_map_header(reader.read_data());
            verify(map.header.version == MAP_FORMAT_VERSION);
            break;

        case ChunkId::map_data:
            verify(!reader.has_data());
            reader.open();
            read_map_data(map, reader);
            reader.close();
            break;
        }
    }
    return map;
}

} // namespace openglyph::io
