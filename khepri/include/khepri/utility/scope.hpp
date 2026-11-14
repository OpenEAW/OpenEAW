#pragma once

#include <type_traits>

namespace khepri {

template <typename ExitFunc>
class ScopeExit
{
public:
    explicit ScopeExit(ExitFunc&& f) noexcept(std::is_nothrow_constructible_v<ExitFunc, ExitFunc&>)
    try : m_exit_func(f) {
    } catch (...) {
        f();
    }

    ScopeExit(ScopeExit&& other) noexcept(std::is_nothrow_copy_constructible_v<ScopeExit>)
        : m_exit_func(other.m_exit_func)
    {
        other.release();
    }

    ScopeExit(const ScopeExit&)            = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit& operator=(ScopeExit&&)      = delete;

    ~ScopeExit() noexcept
    {
        if (!m_released) {
            m_exit_func();
        }
    }

    void release() noexcept
    {
        m_released = true;
    }

private:
    ExitFunc m_exit_func;
    bool     m_released{false};
};

template <typename ExitFunc>
ScopeExit(ExitFunc) -> ScopeExit<ExitFunc>;

} // namespace khepri