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
};

#endif // EVENT_HPP
