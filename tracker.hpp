#ifndef TRACKER_HPP
#define TRACKER_HPP

#include <QMainWindow>
#include <QSettings>
#include <QMap>
#include <QList>

#include "event.hpp"
#include "label.hpp"

class QMainWindow;
class QRect;
class QNetworkReply;
class QNetworkAccessManager;
class QToolBar;
class QSettings;
class QComboBox;
class QTimer;
class QFont;

class Tracker : public QMainWindow {
    Q_OBJECT
public:
    explicit Tracker(QMainWindow* parent=0);
    ~Tracker();

    void getJsonData();
    void init(const QRect& desktop);
    void firstTimeSetup(const QRect& desktop);
    void saveSettings();
    void loadSettings();
    void addEvents();

public slots:
    void replyFinished(QNetworkReply* reply);
    void openOptions();
    void updateEvents();
    
private:
    QNetworkAccessManager* _manager;
    QToolBar*              _toolbar;

    // ["map_names", "event_names"][id] = name
    // ["world_names"][name] = id
    QMap<QString, QMap<QString, QString>> _json;

    // [id] = state, map_id
    QMap<QString, EventState*>            _eventStates;
    QList<ClickableLabel*>                _events;
    QList<QList<QString>>                 _wishlist;

    QString                _baseUrl = "https://api.guildwars2.com";
    QString                _worldId;
    QString                _language; // en, de, fr, es
    QString                _settingsPath;
    QSettings              _settings;
    int                    _updateInterval = 30; // in seconds
    int                    _requestCounter;
    QComboBox*             _servComboBox;
    QComboBox*             _langComboBox;
    QTimer*                _timer;
    QFont                  _font;
    QMap<QString, QString> _eventColors;
};

#endif // TRACKER_HPP
