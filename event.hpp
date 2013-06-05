#ifndef EVENT_HPP
#define EVENT_HPP

#include <QString>
#include <QStringList>

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

struct EventChain : QStringList {
    bool        tracked;

    inline EventChain()
        : QStringList(), tracked(false)
    {
    }

    inline EventChain(bool tracked, const QStringList& events)
        : QStringList(events), tracked(tracked)
    {
    }
};

#endif // EVENT_HPP
