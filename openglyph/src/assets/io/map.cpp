#include <khepri/io/exceptions.hpp>
#include <khepri/io/serialize.hpp>
#include <khepri/math/quaternion.hpp>
#include <khepri/math/serialize.hpp>

#include <openglyph/assets/io/map.hpp>
#include <openglyph/io/chunk_reader.hpp>

#include <cstdint>

namespace openglyph::io {
namespace {
enum MapChunkId : std::uint16_t
{
    map_info = 0x00,
    map_data = 0x01,

    map_data_environment_set    = 0x100,
    map_data_environments       = 0x04,
    map_data_environment        = 0x06,
    map_data_active_environment = 0x08,

    map_data_objects     = 0x102,
    map_data_object_list = 1,
    map_data_object      = 0x44c,
    map_data_object_id   = 0x454,
    map_data_object_data = 0x459,
    map_data_object_core = 0x4b0,
};

// Only supported map version
constexpr std::uint32_t MAP_FORMAT_VERSION = 0x201;

void verify(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}

auto as_string(gsl::span<const uint8_t> data)
{
    const auto* const end = std::find(data.begin(), data.end(), 0);
    return std::string{data.begin(), end};
}

auto as_float(gsl::span<const uint8_t> data)
{
    verify(data.size() == sizeof(float));
    return khepri::io::Deserializer(data).read<float>();
}

auto as_uint32(gsl::span<const uint8_t> data)
{
    verify(data.size() == sizeof(std::uint32_t));
    return khepri::io::Deserializer(data).read<std::uint32_t>();
}

auto as_vector3(gsl::span<const uint8_t> data)
{
    verify(data.size() == sizeof(khepri::Vector3f));
    return khepri::io::Deserializer(data).read<khepri::Vector3f>();
}

auto as_rgb_color(gsl::span<const uint8_t> data)
{
    verify(data.size() == sizeof(khepri::ColorRGB));
    return khepri::io::Deserializer(data).read<khepri::ColorRGB>();
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
        default:
            break;
        }
    }
    return header;
}

auto read_map_environment(gsl::span<const std::uint8_t> data)
{
    Environment     environment;
    MinichunkReader reader(data);

    // Angles where the lights are coming from, in radians. Note that the z-angles stored in the map
    // do NOT have 0° at +X, but at -Y.
    // By default: point to -Y, thus lighting the 'front' (-Y) of objects if not rotated.
    std::array<float, 3> light_zangles = {0, 0, 0}; // CCW angle from -Y.
    std::array<float, 3> light_tilts   = {0, 0, 0}; // Angle above/below XY plane.

    // Wind angle, in degrees
    float wind_zangle = 0;

    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case 0:
            environment.lights[0].color = as_rgb_color(reader.read_data());
            break;
        case 1:
            environment.lights[1].color = as_rgb_color(reader.read_data());
            break;
        case 2:
            environment.lights[2].color = as_rgb_color(reader.read_data());
            break;
        case 3:
            environment.lights[0].specular_color = as_rgb_color(reader.read_data());
            break;
        case 4:
            environment.ambient_color = as_rgb_color(reader.read_data());
            break;
        case 5:
            environment.lights[0].intensity = as_float(reader.read_data());
            break;
        case 6:
            environment.lights[1].intensity = as_float(reader.read_data());
            break;
        case 7:
            environment.lights[2].intensity = as_float(reader.read_data());
            break;
        case 8:
            // Lighting angles are stored in radians
            light_zangles[0] = as_float(reader.read_data());
            break;
        case 9:
            light_zangles[1] = as_float(reader.read_data());
            break;
        case 10:
            light_zangles[2] = as_float(reader.read_data());
            break;
        case 11:
            light_tilts[0] = as_float(reader.read_data());
            break;
        case 12:
            light_tilts[1] = as_float(reader.read_data());
            break;
        case 13:
            light_tilts[2] = as_float(reader.read_data());
            break;
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
            // Skydome angles are stored in degrees
            environment.skydomes[0].tilt = khepri::to_radians(as_float(reader.read_data()));
            break;
        case 30:
            environment.skydomes[1].tilt = khepri::to_radians(as_float(reader.read_data()));
            break;
        case 31:
            // Note:
            environment.skydomes[0].z_angle = khepri::to_radians(as_float(reader.read_data()));
            break;
        case 32:
            environment.skydomes[1].z_angle = khepri::to_radians(as_float(reader.read_data()));
            break;
        case 43:
            // Wind angle is stored in degrees
            wind_zangle = as_float(reader.read_data());
        case 44:
            environment.wind.speed = as_float(reader.read_data());
            break;
        default:
            break;
        }
    }

    // Convert some stored representations into more-easily used versions (e.g. angles into vectors)
    // Note that z-angles in the map have 0° at -Y, but still go counter-clockwise (viewed from +Z).
    for (int i = 0; i < Environment::NUM_LIGHTS; ++i) {
        environment.lights[i].from_direction =
            khepri::Vector3f::from_angles(light_tilts[i], light_zangles[i] - 90);
    }
    environment.wind.to_direction = khepri::Vector2f::from_angle(khepri::to_radians(wind_zangle));

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
        default:
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
        case MapChunkId::map_data_environment:
            verify(reader.has_data());
            environments.push_back(read_map_environment(reader.read_data()));
            break;
        default:
            break;
        }
    }
    return environments;
}

auto read_map_environment_set(Map& map, ChunkReader& reader)
{
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case MapChunkId::map_data_environments:
            verify(!reader.has_data());
            reader.open();
            map.environments = read_map_environments(reader);
            reader.close();
            break;
        case MapChunkId::map_data_active_environment:
            verify(reader.has_data());
            map.active_environment = read_active_environment(reader.read_data());
            break;
        default:
            break;
        }
    }

    if (map.active_environment >= map.environments.size()) {
        map.active_environment = 0;
    }
}

Map::Object read_map_object(Map& map, ChunkReader& reader)
{
    Map::Object object;
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case MapChunkId::map_data_object_id: {
            MinichunkReader minireader(reader.read_data());
            for (; minireader.has_chunk(); minireader.next()) {
                switch (reader.id()) {
                case 0:
                    object.id = as_uint32(minireader.read_data());
                    break;
                default:
                    break;
                }
            }
            break;
        }

        case MapChunkId::map_data_object_data:
            verify(!reader.has_data());
            reader.open();
            for (; reader.has_chunk(); reader.next()) {
                switch (reader.id()) {
                case MapChunkId::map_data_object_core: {
                    MinichunkReader minireader(reader.read_data());
                    for (; minireader.has_chunk(); minireader.next()) {
                        switch (minireader.id()) {
                        case 1:
                            object.type_crc = as_uint32(minireader.read_data());
                            break;
                        case 4:
                            object.position = as_vector3(minireader.read_data());
                            break;
                        case 18: {
                            const auto angles = as_vector3(minireader.read_data());
                            object.facing     = khepri::Quaternionf::from_euler(
                                khepri::to_radians(angles.x), khepri::to_radians(angles.y),
                                khepri::to_radians(angles.z), khepri::ExtrinsicRotationOrder::zyx);
                            break;
                        }
                        default:
                            break;
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }
            reader.close();
            break;

        default:
            break;
        }
    }
    return object;
}

void read_map_objects(Map& map, ChunkReader& reader)
{
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case MapChunkId::map_data_object:
            verify(!reader.has_data());
            reader.open();
            map.objects.push_back(read_map_object(map, reader));
            reader.close();
        default:
            break;
        }
    }
}

void read_map_data(Map& map, ChunkReader& reader)
{
    for (; reader.has_chunk(); reader.next()) {
        switch (reader.id()) {
        case MapChunkId::map_data_environment_set:
            verify(!reader.has_data());
            reader.open();
            read_map_environment_set(map, reader);
            reader.close();
            break;
        case MapChunkId::map_data_objects:
            verify(!reader.has_data());
            reader.open();
            for (; reader.has_chunk(); reader.next()) {
                switch (reader.id()) {
                case MapChunkId::map_data_object_list:
                    verify(!reader.has_data());
                    reader.open();
                    read_map_objects(map, reader);
                    reader.close();
                    break;
                default:
                    break;
                }
            }
            reader.close();
            break;
        default:
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
        case MapChunkId::map_info:
            verify(reader.has_data());
            map.header = read_map_header(reader.read_data());
            verify(map.header.version == MAP_FORMAT_VERSION);
            break;

        case MapChunkId::map_data:
            verify(!reader.has_data());
            reader.open();
            read_map_data(map, reader);
            reader.close();
            break;

        default:
            break;
        }
    }
    return map;
}

} // namespace openglyph::io
