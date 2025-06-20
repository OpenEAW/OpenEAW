#include <khepri/application/console_logger.hpp>
#include <khepri/log/log.hpp>

#ifdef _MSC_VER
#include <Windows.h>
#endif

#include <cassert>
#include <cstdio>
#include <iterator>

namespace khepri::application {

class BaseLogger : private log::Sink
{
public:
    BaseLogger()
    {
        log::add_sink(this);
        m_log_start = log::Clock::now();
    }

    BaseLogger(const BaseLogger&)            = delete;
    BaseLogger(BaseLogger&&)                 = delete;
    BaseLogger& operator=(const BaseLogger&) = delete;
    BaseLogger& operator=(BaseLogger&&)      = delete;

    ~BaseLogger() noexcept override
    {
        log::remove_sink(this);
    }

protected:
    virtual void do_write(std::string_view data) noexcept = 0;

private:
    void write(const log::RecordView& record) noexcept override
    {
        constexpr int MS_PER_S = 1000;

        const auto time_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(record.timestamp - m_log_start)
                .count();

        try {
            fmt::memory_buffer out;
            fmt::format_to(std::back_inserter(out), FMT_STRING("+{}.{:#03d} {}/{}: {}\n"),
                           time_ms / MS_PER_S, time_ms % MS_PER_S, get_char(record.severity),
                           record.logger, record.message);
            do_write({out.data(), out.size()});
        } catch (const fmt::format_error&) {
            assert(false);
        }
    }

    static char get_char(log::Severity severity) noexcept
    {
        switch (severity) {
        case log::Severity::critical:
            return 'C';
            break;
        case log::Severity::error:
            return 'E';
            break;
        case log::Severity::warning:
            return 'W';
            break;
        case log::Severity::info:
            return 'I';
            break;
        case log::Severity::debug:
            return 'D';
            break;
        }
        assert(false);
        return '?';
    }

    log::Clock::time_point m_log_start;
};

#ifdef _MSC_VER
class ConsoleLogger::Impl final : private BaseLogger
{
public:
    Impl()
        // Logging goes to stderr by convention
        : m_owns_console(AllocConsole() != FALSE), m_output_handle(GetStdHandle(STD_ERROR_HANDLE))
    {
    }

    Impl(const Impl&)            = delete;
    Impl(Impl&&)                 = delete;
    Impl& operator=(const Impl&) = delete;
    Impl& operator=(Impl&&)      = delete;

    ~Impl() noexcept override
    {
        if (m_owns_console) {
            FreeConsole();
        }
    }

protected:
    void do_write(std::string_view data) noexcept override
    {
        DWORD written = 0;
        WriteConsole(m_output_handle, data.data(), static_cast<DWORD>(data.size()), &written,
                     nullptr);
    }

private:
    bool   m_owns_console;
    HANDLE m_output_handle;
};
#else
class ConsoleLogger::Impl final : private BaseLogger
{
protected:
    void do_write(std::string_view data) noexcept override
    {
        fwrite(data.data(), data.size(), 1, stderr);
    }
};
#endif

ConsoleLogger::ConsoleLogger() : m_impl(std::make_unique<Impl>()) {}

ConsoleLogger::~ConsoleLogger() = default;

} // namespace khepri::application
