#ifndef EVENT_HPP
#define EVENT_HPP

#include <QString>

struct Event {
    QString map_id;
    QString state;

    inline Event(QString map_id, QString state)
        : map_id(map_id), state(state)
    {
    }
};

#endif // EVENT_HPP