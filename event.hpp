#ifndef EVENT_HPP
#define EVENT_HPP

#include <QString>

struct EventState {
    QString map_id;
    QString state;

    inline EventState(const QString& map_id, const QString& state)
        : map_id(map_id), state(state)
    {
    }

    inline EventState(const EventState& other)
        : map_id(other.map_id), state(other.state)
    {
    }
};

#endif // EVENT_HPP
