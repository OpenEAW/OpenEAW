#pragma once

#include <memory>

namespace khepri::application {

/**
 * \brief A logger that outputs to the console
 *
 * This logger registers itself as a sink for the logging system and outputs all records to the
 * console. If the application is already associated with a console, use that console. Otherwise,
 * it creates a new console.
 */
class ConsoleLogger final
{
public:
    ConsoleLogger();
    ConsoleLogger(const ConsoleLogger&) = delete;
    ConsoleLogger(ConsoleLogger&&)      = delete;
    ConsoleLogger& operator=(const ConsoleLogger&) = delete;
    ConsoleLogger& operator=(ConsoleLogger&&) = delete;
    ~ConsoleLogger();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace khepri::application