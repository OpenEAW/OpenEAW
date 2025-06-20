#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string_view>

namespace khepri::log {

// Use steady_clock to avoid changes in system time from
// messing up the log timestamps.
using Clock = std::chrono::steady_clock;

enum class Severity
{
    debug,
    info,
    warning,
    error,
    critical,
};

/**
 * \brief A non-owning view on a log record.
 *
 * This view represents a log record and is passed to sinks to be output.
 */
struct RecordView
{
    /// The name of the logger where the record originated from
    std::string_view logger;

    /// The time when the log record was created
    Clock::time_point timestamp;

    /// The severity of the log record
    Severity severity;

    /// The formatted message of the log record
    std::string_view message;
};

namespace detail {
void log(const RecordView& record);
}

/**
 * \brief light-weight wrapper around logging functionality
 *
 * This object is a light-weight wrapper around the name passed into the constructor
 * and provides logging functionality. Invoking the various methods on this object
 * will cause a log record to be outputted with the constructed name.
 *
 * For convenience, the message in a log record is formatted with a format string
 * and arguments. The format string must be compatible with the 'fmt' library.
 * For example, the following code will create a message with contents
 * "This is a test message with number 42":
 *
 * \code{.cpp}
 * khepri::log::Logger log("MyLogger");
 * log.debug("This is a {} with number {}", "test message", 42);
 * \endcode
 */
class Logger final
{
public:
    /**
     * Construct the logger
     * \param[in] name the name of the logger
     * \note the memory pointed to by \a name must remain valid for the lifetime of this object.
     */
    constexpr explicit Logger(const char* name) noexcept : m_name(name) {}

    /**
     * Outputs a log record with "debug" severity.
     * \param[in] format the format string (see above)
     * \param[in] args the formatted arguments
     */
    template <typename... TArgs>
    void debug(std::string_view format, TArgs&&... args) const
    {
        log(Severity::debug, format, std::forward<TArgs>(args)...);
    }

    /**
     * Outputs a log record with "info" severity.
     * \param[in] format the format string (see above)
     * \param[in] args the formatted arguments
     */
    template <typename... TArgs>
    void info(std::string_view format, TArgs&&... args) const
    {
        log(Severity::info, format, std::forward<TArgs>(args)...);
    }

    /**
     * Outputs a log record with "warning" severity.
     * \param[in] format the format string (see above)
     * \param[in] args the formatted arguments
     */
    template <typename... TArgs>
    void warning(std::string_view format, TArgs&&... args) const
    {
        log(Severity::warning, format, std::forward<TArgs>(args)...);
    }

    /**
     * Outputs a log record with "error" severity.
     * \param[in] format the format string (see above)
     * \param[in] args the formatted arguments
     */
    template <typename... TArgs>
    void error(std::string_view format, TArgs&&... args) const
    {
        log(Severity::error, format, std::forward<TArgs>(args)...);
    }

    /**
     * Outputs a log record with "critical" severity.
     * \param[in] format the format string (see above)
     * \param[in] args the formatted arguments
     */
    template <typename... TArgs>
    void critical(std::string_view format, TArgs&&... args) const
    {
        log(Severity::critical, format, std::forward<TArgs>(args)...);
    }

    /**
     * Outputs a log record with custom severity
     * \param[in] severity the severity of the log record
     * \param[in] format the format string (see above)
     * \param[in] args the formatted arguments
     */
    template <typename... TArgs>
    void log(Severity severity, std::string_view format, TArgs&&... args) const
    {
        detail::log(
            {m_name, Clock::now(), severity, fmt::format(format, std::forward<TArgs>(args)...)});
    }

private:
    const char* m_name;
};

/**
 * \brief A output for log records
 *
 * Log records are sent to sink, which can output them, store them, etc.
 */
class Sink
{
public:
    Sink()          = default;
    virtual ~Sink() = default;

    Sink(const Sink&)            = delete;
    Sink(Sink&&)                 = delete;
    Sink& operator=(const Sink&) = delete;
    Sink& operator=(Sink&&)      = delete;

    /**
     * Called by the logging system to write a log record to the sink.
     * \param[in] record the record to write
     */
    virtual void write(const RecordView& record) noexcept = 0;
};

/**
 * \brief Adds a sink to the logging system.
 * \param[in] sink the sink to add
 * \note the object pointed to by \a sink must remain alive for the duration of the program,
 *       or until removed with \a remove_sink
 *
 * Any logs created via a \ref logger, are sent to all registered sinks.
 */
void add_sink(Sink* sink);

/**
 * \brief Removes a sink from the logging system.
 * \param[in] sink the sink to remove
 */
void remove_sink(Sink* sink);

} // namespace khepri::log
