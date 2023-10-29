#ifndef RYME_EVENT_HPP
#define RYME_EVENT_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Map.hpp>
#include <Ryme/NonCopyable.hpp>

#include <Ryme/ThirdParty/python.hpp>

namespace ryme {

class RYME_API EventData
{
public:

    template <class T>
    const T * GetAs() const {
        return dynamic_cast<const T *>(this);
    }

    virtual py::object GetPyObject() const = 0;

private:

}; // class EventData

template <class EventDataType>
class RYME_API Event : public NonCopyable
{

    static_assert(
        std::is_base_of<EventData, EventDataType>::value,
        "EventDataType must be a descendant of EventData"
    );

public:

    Event() = default;

    virtual ~Event() = default;

    typedef std::function<void(const EventDataType *)> Listener;

    inline unsigned AddListener(Listener listener) {
        _listenerMap.emplace(_nextListenerId, ListenerRecord{
            .isPython = false,
            .listener = listener,
        });
        return _nextListenerId++;
    }

    inline unsigned AddListener(py::object pyListener) {
        _listenerMap.emplace(_nextListenerId, ListenerRecord{
            .isPython = true,
            .pyListener = pyListener,
        });
        pyListener.inc_ref();
        return _nextListenerId++;
    }

    inline bool RemoveListener(unsigned id) {
        auto it = _listenerMap.find(id);
        if (it != _listenerMap.end()) {
            _listenerMap.erase(it);
            return true;
        }

        return false;
    }

    inline bool RemoveListener(Listener listener) {
        for (const auto& [key, value] : _listenerMap) {
            if (not value.isPython && value.listener == listener) {
                _listenerMap.erase(key);
                return true;
            }        }

        return false;
    }

    inline bool RemoveListener(py::object pyListener) {
        for (const auto& [key, value] : _listenerMap) {
            if (value.isPython && value.pyListener == pyListener) {
                _listenerMap.erase(key);
                return true;
            }
        }

        return false;
    }

    inline void Call(const EventDataType * data) {
        for (auto& [key, value] : _listenerMap) {
            if (value.isPython) {
                value.pyListener.call(data->GetPyObject());
            }
            else {
                value.listener(data);
            }
        }
    }

    void RemoveAllListeners() {
        for (const auto& [key, value] : _listenerMap) {
            if (value.isPython) {
                value.pyListener.dec_ref();
            }
        }
        _listenerMap.clear();
    }

private:

    struct ListenerRecord {

        bool isPython;

        union {
            Listener listener;
            py::object pyListener;
        };

    }; // struct ListenerRecord

    unsigned _nextListenerId = 0;

    Map<unsigned, ListenerRecord> _listenerMap;

}; // class Event

} // namespace ryme

#endif // RYME_EVENT_HPP