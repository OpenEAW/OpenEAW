#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace khepri {

template <typename>
class Slot;

/**
 * A receiver for events sent by a \ref Signal
 */
template <typename... Args>
class Slot<void(Args...)>
{
public:
    /**
     * Constructs a slot from a generic callable
     */
    template <typename Callable>
    Slot(const Callable& callable) : m_func(callable)
    {
    }

    /// Invokes the slot
    void invoke(Args&&... args) const
    {
        m_func(std::forward<Args>(args)...);
    }

private:
    std::function<void(Args...)> m_func;
};

namespace detail {

class ConnectionBase
{
public:
    [[nodiscard]] bool connected() const noexcept
    {
        return m_connected;
    }

    void disconnect() noexcept
    {
        m_connected = false;
    }

private:
    bool m_connected{true};
};

template <typename>
class ConnectionImpl;

// Shared instance that tracks everything for a connection between a signal and slot
template <typename... Args>
class ConnectionImpl<void(Args...)> : public ConnectionBase
{
    using SlotType = Slot<void(Args...)>;

public:
    explicit ConnectionImpl(SlotType slot) : m_slot(std::move(slot)) {}

    bool invoke_slot(Args&&... args) const
    {
        if (!connected()) {
            return false;
        }
        m_slot.invoke(std::forward<Args>(args)...);
        return true;
    }

private:
    SlotType m_slot;
};
} // namespace detail

/**
 * Represents a connection between a signal and a slot.
 *
 * A connection is a light-weight value type that can be used to discover if its slot and signal are
 * still connected, and to disconnect them.
 *
 * A connection between a signal and slot can be disconnected if the signal is destructed, or if a
 * connection object is used to disconnect them.
 */
class Connection
{
    template <typename>
    friend class Signal;

public:
    /// Constructs a disconnected connection
    Connection() = default;

    /**
     * Disconnects the signal and slot associated with the connection. If the referenced slot is
     * not connected, this operation is a no-op.
     *
     * Once disconnected, the associated slot will no longer receive events from the associated
     * signal via this connection (it's possible that multiple connections between a signal and
     * slot exist).
     */
    void disconnect() noexcept
    {
        if (auto impl = m_impl.lock()) {
            impl->disconnect();
            m_impl = {};
        }
    }

    /**
     * Returns whether the connection is still active, that is, the associated slot is still
     * connected to the associated signal.
     */
    [[nodiscard]] bool connected() const noexcept
    {
        if (auto impl = m_impl.lock()) {
            return impl->connected();
        }
        return false;
    }

private:
    explicit Connection(std::weak_ptr<detail::ConnectionBase> impl) : m_impl(std::move(impl)) {}

    std::weak_ptr<detail::ConnectionBase> m_impl;
};

/**
 *  A connection which is automatically disconnected on destruction.
 */
class ScopedConnection final : public Connection
{
public:
    /// Constructs a disconnected scoped connection
    ScopedConnection() noexcept = default;

    /// Constructs a scoped connection for a given connection
    explicit ScopedConnection(const Connection& conn) noexcept : Connection(conn) {}

    ScopedConnection(const ScopedConnection& conn)       = delete;
    ScopedConnection& operator=(const ScopedConnection&) = delete;

    /**
     * Constructs a scoped connection by moving from another scoped connection.
     * After construction, \a conn no longer references any connection.
     */
    ScopedConnection(ScopedConnection&& conn) noexcept : Connection(std::move(conn)) {}

    /**
     * Constructs a scoped connection by moving from another connection.
     * After construction, \a conn is a disconnected connection.
     */
    ScopedConnection(Connection&& conn) noexcept : Connection(std::move(conn)) {}

    /**
     * Disconnects the signal and slot associated with the \a scoped_connection. If the referenced
     * slot is not connected, this operation is a no-op.
     */
    ~ScopedConnection() noexcept
    {
        disconnect();
    }

    /**
     * Copy assignment from unscoped connection.
     *
     * If this \a scoped_connection already referenced another connection, it will be disconnected
     * first.
     */
    ScopedConnection& operator=(const Connection& conn) noexcept
    {
        disconnect();
        static_cast<Connection&>(*this) = conn;
    }

    /**
     * Move assignment
     *
     * If this \a scoped_connection already referenced another connection, it will be disconnected
     * first. After assignment, \a conn no longer references any connection.
     */
    ScopedConnection& operator=(ScopedConnection&& conn) noexcept
    {
        disconnect();
        static_cast<Connection&>(*this) = std::move(conn);
    }

    /**
     * Move assignment
     *
     * If this \a scoped_connection already referenced another connection, it will be disconnected
     * first. After assignment, \a conn is a disconnected connection.
     */
    ScopedConnection& operator=(Connection&& conn) noexcept
    {
        disconnect();
        static_cast<Connection&>(*this) = std::move(conn);
    }

    /// Releases the connection so it will not be disconnected on destruction
    Connection release() noexcept
    {
        Connection conn                 = *this;
        static_cast<Connection&>(*this) = {};
        return conn;
    }
};

template <typename>
class Signal;

/**
 * An event-broadcast signal.
 *
 * Signals are objects that send events to attached slots.
 */
template <typename... Args>
class Signal<void(Args...)>
{
    using ConnectionImpl = detail::ConnectionImpl<void(Args...)>;

public:
    /// The associated slot type for this signal
    using SlotType = Slot<void(Args...)>;

    /**
     * Connects this signal to a slot.
     *
     * After connecting, the slot will receive all events sent by the signal.
     */
    auto connect(SlotType slot)
    {
        auto conn = std::make_shared<ConnectionImpl>(std::move(slot));
        m_connections.push_back(conn);
        return Connection(std::move(conn));
    }

    /**
     * Send an event to all connected slots
     */
    void operator()(Args&&... args) const
    {
        // Remove all connections whose invoke returns false (which indicates they are dead)
        m_connections.erase(
            std::remove_if(
                m_connections.begin(), m_connections.end(),
                [&](const auto& impl) { return !impl->invoke_slot(std::forward<Args>(args)...); }),
            m_connections.end());
    }

private:
    mutable std::vector<std::shared_ptr<ConnectionImpl>> m_connections;
};

} // namespace khepri