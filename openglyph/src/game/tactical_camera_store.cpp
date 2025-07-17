#include <khepri/game/rts_camera.hpp>
#include <khepri/log/log.hpp>
#include <khepri/math/math.hpp>

#include <openglyph/game/tactical_camera_store.hpp>
#include <openglyph/parser/parsers.hpp>

#include <algorithm>
#include <utility>

namespace openglyph {
namespace {
constexpr khepri::log::Logger LOG("tactical_cameras");

template <typename T>
T identity(const T& value) noexcept
{
    return value;
}

template <typename UnitConverter>
std::vector<khepri::Point> convert_unit(gsl::span<const khepri::Point> points,
                                        const UnitConverter&           unit_converter)
{
    if (unit_converter == &identity<double>) {
        return std::vector<khepri::Point>(points.begin(), points.end());
    }
    std::vector<khepri::Point> converted_points(points.size());
    std::transform(points.begin(), points.end(), converted_points.begin(),
                   [&](const auto& p) { return khepri::Point{p.x, unit_converter(p.y)}; });
    return converted_points;
}

} // namespace

// Describes a tactical camera
struct TacticalCameraStore::TacticalCamera
{
    using RtsCameraController = khepri::game::RtsCameraController;

    struct FreeProperty
    {
        khepri::Range constraint;
        double        sensitivity;
        double        smooth_time;
    };

    struct ZoomProperty
    {
        std::vector<khepri::Point> points; // Points for the interpolator
        double                     smooth_time;
    };

    using PitchProperty = std::variant<FreeProperty, ZoomProperty>;

    std::string name;

    // Note: all angles are in radians, and all distances are in world units.
    PitchProperty pitch;
    ZoomProperty  distance;
    ZoomProperty  fov;
    FreeProperty  yaw;

    double zoom_sensitivity{0.1};

    // Near clip plane distance, in world units in front of the camera position
    double near_clip{10.0};

    // Far clip plane distance, in world units in front of the camera position
    double far_clip{10000.0};
};

TacticalCameraStore::TacticalCamera
TacticalCameraStore::read_tactical_camera(const XmlParser::Node& node)
{
    using RtsCameraController = khepri::game::RtsCameraController;

    const auto read_free_property = [&](const std::string& name, const auto& unit_converter) {
        auto min = unit_converter(parse<double>(optional_child(node, name + "_Min", "0")));
        auto max = unit_converter(parse<double>(optional_child(node, name + "_Max", "0")));
        auto pmu = parse<double>(optional_child(node, name + "_Per_Mouse_Unit", "1"));
        if (min > max) {
            std::swap(min, max);
        }
        return TacticalCamera::FreeProperty{{min, max}, pmu};
    };

    const auto read_zoom_property = [&](const std::string& name, bool use_spline,
                                        const auto& unit_converter) {
        TacticalCamera::ZoomProperty property;
        property.smooth_time = parse<double>(optional_child(node, name + "_Smooth_Time", "0.1"));
        if (use_spline) {
            // Property uses zoom-based splines.
            property.points = convert_unit(
                parse<khepri::CubicInterpolator>(optional_child(node, name + "_Spline", ""))
                    .points(),
                unit_converter);
        } else {
            // Property is a range (but has no sensitivity or default value, because it depends on
            // zoom). We can create a linear interpolator from the min and max values.
            auto min = unit_converter(parse<double>(optional_child(node, name + "_Min", "0")));
            auto max = unit_converter(parse<double>(optional_child(node, name + "_Max", "0")));
            if (min > max) {
                std::swap(min, max);
            }
            property.points = {{0, min}, {1, max}};
        }
        return property;
    };

    const auto read_pitch_property =
        [&](const std::string& name, bool use_spline,
            const auto& unit_converter) -> TacticalCamera::PitchProperty {
        if (use_spline) {
            // Property uses zoom-based splines.
            return read_zoom_property(name, use_spline, unit_converter);
        }
        // Property is a free property.
        return read_free_property(name, unit_converter);
    };

    TacticalCamera camera;

    const auto use_splines = parse<bool>(optional_child(node, "Use_Splines", "false"));
    if (use_splines) {
        camera.zoom_sensitivity =
            1.0 / parse<unsigned int>(optional_child(node, "Spline_Steps", "10"));
    }

    camera.name      = std::string(require_attribute(node, "Name"));
    camera.pitch     = read_pitch_property("Pitch", use_splines, &khepri::to_radians<double>);
    camera.distance  = read_zoom_property("Distance", use_splines, &identity<double>);
    camera.fov       = read_zoom_property("Fov", false, &khepri::to_radians<double>);
    camera.yaw       = read_free_property("Yaw", &khepri::to_radians<double>);
    camera.near_clip = parse<double>(optional_child(node, "Near_Clip", "0.1"));
    camera.far_clip  = parse<double>(optional_child(node, "Far_Clip", "0.1"));

    // Yaw isn't smoothed in the Glyph engine.
    camera.yaw.smooth_time = 0.0;

    return camera;
}

TacticalCameraStore::TacticalCameraStore(AssetLoader& asset_loader, std::string_view filename)
{
    if (auto stream = asset_loader.open_config(filename)) {
        XmlParser parser(*stream);
        if (const auto& root = parser.root()) {
            for (const auto& node : root->nodes()) {
                try {
                    auto camera = read_tactical_camera(node);
                    auto name   = camera.name;
                    m_tactical_cameras.insert({std::move(name), std::move(camera)});
                } catch (ParseError& e) {
                    LOG.error("Error reading tactical cameras: {}", e.what());
                }
            }
        }
    }
}

TacticalCameraStore::~TacticalCameraStore() = default;

std::optional<khepri::game::RtsCameraController>
TacticalCameraStore::create(std::string_view name, khepri::renderer::Camera& camera) const noexcept
{
    const auto& create_interpolator = [](const std::vector<khepri::Point>& points) {
        return std::make_unique<khepri::CubicInterpolator>(points);
    };

    if (const auto it = m_tactical_cameras.find(name); it != m_tactical_cameras.end()) {
        const auto&                       settings = it->second;
        khepri::game::RtsCameraController rts_camera(camera, {0, 0});
        rts_camera.distance_property(
            {create_interpolator(settings.distance.points), settings.distance.smooth_time});
        rts_camera.fov_property(
            {create_interpolator(settings.fov.points), settings.fov.smooth_time});
        rts_camera.yaw_property(
            {settings.yaw.constraint, settings.yaw.sensitivity, settings.yaw.smooth_time});

        if (const auto* zoom_pitch = std::get_if<TacticalCamera::ZoomProperty>(&settings.pitch)) {
            rts_camera.pitch_property(khepri::game::RtsCameraController::ZoomProperty{
                create_interpolator(zoom_pitch->points), zoom_pitch->smooth_time});
        } else if (const auto* free_pitch =
                       std::get_if<TacticalCamera::FreeProperty>(&settings.pitch)) {
            rts_camera.pitch_property(khepri::game::RtsCameraController::FreeProperty{
                free_pitch->constraint, free_pitch->sensitivity, free_pitch->smooth_time});
        } else {
            assert(false && "Unknown pitch property type");
        }

        camera.znear(settings.near_clip);
        camera.zfar(settings.far_clip);

        // Update the camera to the initial values immediately
        rts_camera.update_immediate();
        return rts_camera;
    }
    return std::nullopt;
}

} // namespace openglyph
