#pragma once

#include <khepri/math/frustum.hpp>
#include <khepri/math/matrix.hpp>
#include <khepri/math/vector3.hpp>

#include <tuple>

namespace khepri::renderer {

/**
 * \brief Represents a camera used for rendering.
 *
 * A camera has a position and an orientation determined by a target to look at and an 'up' vector
 * (a vector in world space that points directly up as seen by the camera).
 *
 * Other camera properties are its Field-of-view, size of the camera surface (indicated by width and
 * aspect ratio), and distance of the near and far clip planes.
 */
class Camera final
{
public:
    /// The type of camera
    enum class Type
    {
        orthographic,
        perspective,
    };

    /// The camera properties
    struct Properties
    {
        Type    type{};     ///< The type of the camera
        Vector3 position{}; ///< The world-space position of the camera
        Vector3 target{};   ///< The world-space vector of the target of the camera
        Vector3 up{};       ///< The world-space vector corresponding to 'up' on the camera
        double  fov{};      ///< Vertical field of view in radians (perspective cameras only)
        double  width{};    ///< Width, in world units, of the camera (orthographic cameras only)
        double  aspect{};   ///< Aspect ratio (Width / Height) of the render viewport
        double  znear{};    ///< Distance, in camera-space units, of the near clip plane
        double  zfar{};     ///< Distance, in camera-space units, of the far clip plane
    };

    /// Collection of useful matrices derived from the camera properties
    struct Matrices
    {
        Matrixf view{};          ///< World-to-Camera-space matrix
        Matrixf view_inv{};      ///< inverse of m_view
        Matrixf projection{};    ///< Camera-to-Screen-space matrix
        Matrixf view_proj{};     ///< m_view * m_proj
        Matrixf view_proj_inv{}; ///< inverse of m_view_proj
    };

    /**
     * Constructs the camera
     * \param[in] properties the properties for the new camera
     */
    explicit Camera(const Properties& properties);

    /// Returns the current properties of the camera
    [[nodiscard]] const auto& properties() const noexcept
    {
        return m_properties;
    }

    /// Returns the matrices derived from the camera properties
    [[nodiscard]] const auto& matrices() const noexcept
    {
        return m_matrices;
    }

    /// Returns the type of the camera
    [[nodiscard]] auto type() const noexcept
    {
        return m_properties.type;
    }

    /// Returns the position of the camera
    [[nodiscard]] const Vector3& position() const noexcept
    {
        return m_properties.position;
    }

    /// Returns the target of the camera
    [[nodiscard]] const Vector3& target() const noexcept
    {
        return m_properties.target;
    }

    /// Returns the up vector of the camera
    [[nodiscard]] const Vector3& up() const noexcept
    {
        return m_properties.up;
    }

    /// Returns the field-of-view angle of the camera (radians)
    [[nodiscard]] double fov() const noexcept
    {
        return m_properties.fov;
    }

    /// Returns the width of the camera surface (in world units)
    [[nodiscard]] double width() const noexcept
    {
        return m_properties.width;
    }

    /// Returns the aspect ratio of the camera surface (height / width)
    [[nodiscard]] double aspect() const noexcept
    {
        return m_properties.aspect;
    }

    /// Returns the distance from the camera position to the near plane (in world units)
    [[nodiscard]] double znear() const noexcept
    {
        return m_properties.znear;
    }

    /// Returns the distance from the camera position to the far plane (in world units)
    [[nodiscard]] double zfar() const noexcept
    {
        return m_properties.zfar;
    }

    /**
     * \brief Sets the new properties for the camera
     * \param[in] properties the new properties
     *
     * This method updates all properties of the camera and recalculates the matrices.
     */
    void properties(const Properties& properties) noexcept;

    /**
     * \brief Computes the level-of-detail for a position in the world
     * \param[in] world_pos the world position to compute the LOD for
     * \return the LOD (range 0 - 1) for \a world_pos
     */
    [[nodiscard]] double lod(const Vector3& world_pos) const noexcept;

    /**
     * \brief Unprojects a 2D point on the camera surface to two 3D points.
     *
     * Unprojects two normalized device coordinates ([-1,+1] range) on the camera surface to two
     * vectors defining the line that that point makes through the scene. The two returned vectors
     * represent the unprojected point on the near and far view plane, respectively.

     * \param[in] coords the 2D coordinate to unproject
     * \return the unprojected coordinate on the near and far plane
     */
    [[nodiscard]] std::tuple<Vector3, Vector3> unproject(const Vector2& coords) const noexcept;

    /**
     * Returns the view frustum for the entire camera
     */
    [[nodiscard]] const auto& frustum() const noexcept
    {
        return m_frustum;
    }

    /**
     * \brief Returns the view frustum for a subsection of the camera
     *
     * Returns a view frustum for a sub-set of the camera surface defined by
     * the box defined by the corners \a p1 and \a p2, in normalized device
     * coordinates ([-1,+1] range).
     *
     * \param[in] p1 the top-left point on the camera surface
     * \param[in] p2 the bottom-right point on the camera surface
     *
     * \return the view frustum for the square identified by \a p1 and \a p2.
     */
    [[nodiscard]] Frustum frustum(const Vector2& p1, const Vector2& p2) const noexcept;

private:
    static Matrices create_matrices(const Properties& properties) noexcept;

    Properties m_properties;
    Matrices   m_matrices;
    Frustum    m_frustum;
};

} // namespace khepri::renderer
