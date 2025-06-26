#include <khepri/log/log.hpp>

#include <mutex>
#include <unordered_set>

namespace khepri::log {

namespace {
class SinkList
{
public:
    void log(const RecordView& record) const
    {
        const std::lock_guard lock(m_sink_mutex);
        for (auto* sink : m_sinks) {
            sink->write(record);
        }
    }

    void add_sink(Sink* sink)
    {
        const std::lock_guard lock(m_sink_mutex);
        m_sinks.insert(sink);
    }

    void remove_sink(Sink* sink)
    {
        const std::lock_guard lock(m_sink_mutex);
        m_sinks.erase(sink);
    }

private:
    mutable std::mutex        m_sink_mutex;
    std::unordered_set<Sink*> m_sinks;
};

SinkList& sinklist()
{
    static SinkList sinklist;
    return sinklist;
}
}; // namespace

namespace detail {

void log(const RecordView& record)
{
    sinklist().log(record);
}

} // namespace detail

void add_sink(Sink* sink)
{
    sinklist().add_sink(sink);
}

void remove_sink(Sink* sink)
{
    sinklist().remove_sink(sink);
}

} // namespace khepri::log
