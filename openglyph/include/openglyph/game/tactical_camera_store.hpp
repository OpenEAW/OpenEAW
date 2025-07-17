#pragma once

#include <khepri/game/rts_camera.hpp>

#include <openglyph/assets/asset_loader.hpp>
#include <openglyph/parser/xml_parser.hpp>

#include <map>
#include <string_view>

namespace openglyph {

/**
 * @brief Loads and stores tactical cameras.
 *
 * The store reads the TacticalCamera definitions and creates RtsCameraController objects for
 * configurations.
 */
class TacticalCameraStore
{
public:
    /**
     * @brief Constructs a GameObjectTypeStore by loading from XML configuration.
     *
     * The AssetLoader is used to load the @a filename configuration file, which is an XML
     * file containing tactical camera definitions.
     *
     * All TacticaCamera objects are created from these definitions and stored in this object.
     */
    TacticalCameraStore(AssetLoader& asset_loader, std::string_view filename);

    TacticalCameraStore(const TacticalCameraStore&)            = delete;
    TacticalCameraStore& operator=(const TacticalCameraStore&) = delete;
    ~TacticalCameraStore();

    /**
     * @brief Create a tactical camera controller by name.
     *
     * @param name the name of the tactical camera to create.
     * @param camera the camera that the tactical camera controller will control.
     *
     * @return a new RtsCameraController object controlling the specified camera if a tactical
     * camera with the specified name exists, or std::nullopt if it does not.
     *
     * @note the name lookup is case insensitive.
     */
    std::optional<khepri::game::RtsCameraController>
    create(std::string_view name, khepri::renderer::Camera& camera) const noexcept;

private:
    struct TacticalCamera;

    TacticalCamera read_tactical_camera(const XmlParser::Node& node);

    std::map<std::string, TacticalCamera, std::less<>> m_tactical_cameras;
};

} // namespace openglyph
