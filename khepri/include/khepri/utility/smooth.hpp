#pragma once

#include <algorithm>
#include <utility>

namespace khepri {

/**
 * \brief A utility class for smoothing value changes over time.
 *
 * This class effectively holds a value of type \a T. Assigning new values to it will not
 * immediately update the value, but do so smoothly over time. To that effect, this class's \a
 * Update() method must be called regularly.
 *
 * The \a smooth_time that can be specified during construction specifies the time, in seconds, that
 * the value lags behind the target value. Passing zero or a negative value for \a smooth_time will
 * cause the value to be updated immediately, and \a Update will not have to be called.
 */
template <typename T>
class Smooth
{
public:
    Smooth() = default;
    Smooth(const T& initial_value) : m_current(initial_value), m_target(initial_value) {}
    Smooth(T&& initial_value) : m_current(initial_value), m_target(std::move(initial_value)) {}
    Smooth(const T& initial_value, double smooth_time)
        : m_current(initial_value), m_target(initial_value), m_smooth_time(smooth_time)
    {
    }
    Smooth(T&& initial_value, double smooth_time)
        : m_current(initial_value), m_target(std::move(initial_value)), m_smooth_time(smooth_time)
    {
    }

    /**
     * \brief Returns the current smooth time.
     */
    double smooth_time() const noexcept
    {
        return m_smooth_time;
    }

    /**
     * \brief Sets the current smooth time. If the smooth time is zero or negative, it also sets the
     * current value to the target immediately.
     */
    void smooth_time(double smooth_time)
    {
        m_smooth_time = smooth_time;
        if (smooth_time < MIN_SMOOTH_TIME) {
            // If the smooth time is too small, set the current value to the target immediately.
            value_immediate(m_target);
        }
    }

    /**
     * \brief Sets the new target for the smoothed value.
     */
    void target(const T& target)
    {
        if (m_smooth_time < MIN_SMOOTH_TIME) {
            // If the smooth time is too small, set the current value to the target immediately.
            value_immediate(target);
        } else {
            m_target = target;
        }
    }

    /**
     * \brief Sets the new target for the smoothed value.
     */
    void target(T&& target)
    {
        if (m_smooth_time < MIN_SMOOTH_TIME) {
            // If the smooth time is too small, set the current value to the target immediately.
            value_immediate(std::move(target));
        } else {
            m_target = std::move(target);
        }
    }

    /**
     * \brief Sets the value of the smoothed value immediately.
     */
    void value_immediate(const T& value)
    {
        m_current  = value;
        m_target   = value;
        m_velocity = T{};
    }

    /**
     * \brief Sets the value of the smoothed value immediately.
     */
    void value_immediate(T&& value)
    {
        m_current  = value;
        m_target   = std::move(value);
        m_velocity = T{};
    }

    /**
     * \brief Updates the value of the smoothed value to approach the target smoothly over time.
     * \param dt The time delta since the last update, in seconds.
     */
    void update(double dt)
    {
        if (m_smooth_time >= MIN_SMOOTH_TIME) {
            // Game Programming Gem 4: "Critically Damped Ease-In/Ease-Out Smoothing".
            const double omega = 2.0 / m_smooth_time;
            const double x     = omega * dt;
            const double exp   = 1.0 / (1.0 + x + 0.48 * x * x + 0.235 * x * x * x);
            const double delta = m_current - m_target;
            const double temp  = (m_velocity + omega * delta) * dt;
            m_velocity         = (m_velocity - omega * temp) * exp;
            m_current          = m_target + (delta + temp) * exp;
        }
    }

    /**
     * \brief Sets the value of the smoothed value to the target immediately, without smoothing.
     */
    void update_immediate()
    {
        m_current  = m_target;
        m_velocity = T{};
    }

    /**
     * \brief Returns the current value of the smoothed value.
     */
    [[nodiscard]] const T& value() const
    {
        return m_current;
    }

    /**
     * \brief Returns the current target of the smoothed value.
     */
    [[nodiscard]] T target() const
    {
        return m_target;
    }

private:
    // Minimum smooth time; any smooth time below this value will cause the value to be updated
    // immediately.
    static constexpr double MIN_SMOOTH_TIME = 0.001;

    T      m_current{};
    T      m_target{};
    T      m_velocity{};
    double m_smooth_time{1.0};
};

} // namespace khepri