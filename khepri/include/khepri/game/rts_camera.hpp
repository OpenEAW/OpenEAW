#pragma once

#include <khepri/math/constraint.hpp>
#include <khepri/math/interpolator.hpp>
#include <khepri/math/range.hpp>
#include <khepri/renderer/camera.hpp>
#include <khepri/utility/smooth.hpp>

#include <functional>
#include <memory>
#include <variant>

namespace khepri::game {

/**
 * \brief An RtsCameraController controls a \see Camera in a way that matches real-time strategy
 * games with a top-down ("bird's eye") camera.
 *
 * An RtsCamera looks at a certain position on the (constrained) XY plane from a certain distance
 * and angle above the plane. It has methods that mirror camera operations (translate, rotate, zoom)
 * that translates the camera across the XY plane, rotate it around the camera's target and zooms it
 * in or out (all while ensuring the camera doesn't clip through objects or terrain, if any).
 *
 * The \a RtsCameraController can also be configured to control certain properties of the camera
 * (e.g. pitch, FoV, distance-to-target) via the controller's zoom level so that zooming in or out
 * changes these properties.
 */
class RtsCameraController final
{
public:
    /**
     * Describes a camera property that is dependent on the camera's zoom level.
     */
    struct ZoomProperty
    {
        /**
         * \brief Describes a interpolator for the property.
         *
         * This interpolator is used to query the property based on the zoom level (0 to 1).
         */
        std::unique_ptr<khepri::Interpolator> interpolator;

        /**
         * How quickly the camera property changes when its value is updated.
         *
         * This is the time, in seconds, it takes for the property to have fully changed from its
         * current value to the new value.
         */
        double smooth_time{0.1};
    };

    /**
     * \brief Describes a camera property that is not dependent on the camera's zoom level.
     */
    struct FreeProperty
    {
        /**
         * The constraint for the property.
         */
        Range constraint;

        /**
         * The sensitivity of the property. Any _relative_ change to the property will be multiplied
         * by this before it is applied.
         */
        double sensitivity{1.0};

        /**
         * How quickly the camera property changes when its value is updated.
         *
         * This is the time, in seconds, it takes for the property to have fully changed from its
         * current value to the new value.
         */
        double smooth_time{0.1};
    };

    /**
     * \brief Describes a constraint for the camera's pitch.
     *
     * Pitch, as the exception, can be configured to be controlled either directly via rotation, or
     * via zoom.
     */
    using PitchProperty = std::variant<FreeProperty, ZoomProperty>;

    /**
     * Constructs an RtsCameraController with default properties.
     *
     * \param[in] camera the camera object that it's controlling.
     * \param[in] target the target position on the XY plane that the camera is looking at.
     *
     * \note the \a camera object must remain alive while this \a RtsCameraController is alive.
     */
    RtsCameraController(renderer::Camera& camera, const Point& target);

    /**
     * @brief Returns the camera's current target on the XY plane.
     */
    const Point& target() const noexcept
    {
        return m_target;
    }

    /**
     * @brief Changes the target of the camera.
     *
     * The new target will still be bound by any configured bounds.
     *
     * @param target the new target position on the XY plane that the camera is looking at.
     */
    void target(const Point& target);

    /**
     * @brief Translates the camera along the XY plane.
     *
     * The translation is specified with a 2D vector where the **direction** of the vector is in
     * **camera space** and the **magnitude** of the specified vector is in **world space**.
     *
     * This means that a translation vector of (2,0) translates the camera across XY towards the
     * right in the current camera view, for a distance of 2 world units.
     *
     * The XY translation is bound by the configured target constraint, if any.
     *
     * \param[in] camera_offset the offset to translate the camera by (direction in camera space,
     *                          magnitude in world space).
     */
    void translate(const Vector2& camera_offset);

    /**
     * @brief Sets the camera's rotation.
     *
     * This effectively changes the yaw and pitch of the camera to the specified values but keeps
     * the distance to the target the same. The camera's new yaw and pitch are bound by their
     * configured constraints.
     *
     * The yaw determines the direction on the XY plane the camera is looking "down". 0 means the
     * camera is looking in the direction of the positive X axis. ½π means the camera is looking
     * in the direction of the positive Y axis.
     *
     * The pitch determines how much the camera is looking down onto the XY plane, ½π means the
     * camera is pointing straight down. 0 means the camera is looking straight across the XY plane.
     *
     * The default yaw and pitch of a newly constructed camera are 0 and ¼π, respectively.
     *
     * @note the @a pitch_angle parameter is ignored if a pitch spline has been set (see \ref
     * pitch_spline).
     *
     * @param yaw_angle   the new yaw angle (radians).
     * @param pitch_angle the new pitch angle (radians).
     */
    void rotation(double yaw_angle, double pitch_angle);

    /**
     * \brief Rotates the camera around the target position.
     *
     * This effectively changes the yaw and pitch of the camera, but keeps the distance to the
     * target the same. The camera's new yaw and pitch are bound by their configured constraints.
     *
     * The arguments to this method are scaled by the \a sensitivity field of the yaw and pitch
     * property, respectively.
     *
     * @note the @a pitch_angle_diff parameter is ignored if pitch is controlled via zoom (see \ref
     * pitch_constraint).
     *
     * \param[in] yaw_angle_diff   angle to change yaw with (radians)
     * \param[in] pitch_angle_diff angle to change pitch with (radians)
     */
    void rotate(double yaw_angle_diff, double pitch_angle_diff);

    /**
     * @brief Returns the current zoom level.
     *
     * The zoom level is clamped between 0.0 (zoomed in) and 1.0 (zoomed out).
     * The zoom level primaruly determines the distance to target and the camera's field-of-view.
     * The zoom level also determines the camera's pitch if so configured (see \ref pitch_property).
     * Otherwise, the pitch is controlled directly via \ref rotation and \ref rotate.
     */
    double zoom_level() const noexcept
    {
        return m_zoom_level;
    }

    /**
     * \brief Sets the zoom level.
     *
     * See \ref zoom_level() for a description of the zoom level.
     *
     * \param[in] level the new zoom level
     */
    void zoom_level(double level);

    /**
     * \brief Zooms the camera in or out.
     *
     * The camera's zoom level is modified by subtracting \a amount multiplied by the the current
     * zoom sensitivity (see \ref zoom_sensitivity).
     *
     * \param[in] amount amount of zoom steps to take. Positive zooms in, negative zooms out.
     */
    void zoom(double amount);

    /**
     * @brief Returns the current zoom sensitivity.
     *
     * The zoom sensitivity is a multiplier for zoom operations. When \ref zoom is called, the zoom
     * amount is first multiplied with the configured zoom sensitivity.
     */
    double zoom_sensitivity() const noexcept
    {
        return m_zoom_sensitivity;
    }

    /**
     * @brief Sets the zoom sensitivity.
     *
     * See \ref zoom_sensitivity() for a description of the zoom sensitivity.
     *
     * @param sensitivity the new zoom sensitivity.
     */
    void zoom_sensitivity(double sensitivity) noexcept;

    /**
     * Sets the camera's property for the distance from its target.
     *
     * The minimum distance is equivalent to the distance in fully zoomed-in state (a zoom level of
     * 0.0). The maximum distance is equivalent to the distance in fully zoomed-out state (a zoom
     * level of 1.0).
     */
    void distance_property(ZoomProperty property);

    /**
     * Sets the camera's property for the camera FoV (field of view).
     *
     * The minimum FoV is equivalent to the FoV in fully zoomed-in state (a zoom level of
     * 0.0). The maximum FoV is equivalent to the FoV in fully zoomed-out state (a zoom
     * level of 1.0).
     */
    void fov_property(ZoomProperty property);

    /**
     * \brief Set the constraint for the target position
     *
     * This constraint is used whenever the camera's target changes to ensure the camera target
     * stays within some bounds. The default constraint is the empty function.
     *
     * When setting a new constraint, the current target is immediately adjusted by the new
     * constraint.
     *
     * \param[in] constraint the new constraint (empty function indicates no bounds).
     */
    void target_constraint(const khepri::Constraint<Point>& constraint);

    /**
     * \brief Sets the constraint for the camera's yaw.
     *
     * This constraint is used whenever the camera's yaw changes to ensure the camera yaw
     * stays within some bounds. The default constraint is the empty function.
     *
     * When setting a new constraint, the current yaw is immediately adjusted by the new
     * constraint.
     *
     * \param[in] constraint the new constraint (empty function indicates no bounds).
     */
    void yaw_property(FreeProperty property);

    /**
     * \brief Sets the constraint for the camera's pitch.
     *
     * Pitch can be controlled either directly via rotation (see \ref rotate) or via zoom.
     * If the constraint is a \ref ZoomConstraint, the pitch is controlled via zoom. Otherwise,
     * the pitch is controlled directly via rotation.
     *
     * The default constraint is the empty function. When setting a new constraint, the current
     * pitch is immediately adjusted by the new constraint.
     *
     * \param[in] constraint the new constraint.
     */
    void pitch_property(PitchProperty property);

    /// Returns the camera's current distance from its target, in world space.
    double distance() const noexcept;

    /// Returns the camera's currently configured yaw (in radians)
    double yaw() const noexcept
    {
        return m_yaw.target();
    }

    /// Returns the camera's currently configured pitch (in radians)
    double pitch() const noexcept
    {
        return m_pitch.target();
    }

    /// Returns the camera's "look at" vector (normalized) in world space.
    Vector3 direction() const noexcept;

    /// Returns the camera's "up" vector (normalized) in world space.
    Vector3 up() const noexcept;

    /// Returns the camera's "right" vector (normalized) in world space.
    Vector3 right() const noexcept;

    /**
     * \brief Updates the camera controller.
     * \param dt The time delta since the last update, in seconds.
     */
    void update(double dt);

    /**
     * Updates the camera properties to their targets immediately, without smoothing.
     */
    void update_immediate();

private:
    static constexpr double DEFAULT_SMOOTH_TIME = 0.1;

    void update_camera() const;

    renderer::Camera& m_camera;

    khepri::Constraint<Point> m_target_constraint;
    ZoomProperty              m_distance_property;
    ZoomProperty              m_fov_property;
    FreeProperty              m_yaw_property;
    PitchProperty             m_pitch_property;

    Point  m_target;
    double m_zoom_sensitivity{0.1};
    double m_zoom_level{0.0};

    Smooth<double> m_distance{10.0, DEFAULT_SMOOTH_TIME};
    Smooth<double> m_fov{PI / 4, DEFAULT_SMOOTH_TIME};
    Smooth<double> m_yaw{0, DEFAULT_SMOOTH_TIME};
    Smooth<double> m_pitch{PI / 4, DEFAULT_SMOOTH_TIME};
};

} // namespace khepri::game
