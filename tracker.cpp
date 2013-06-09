#include <QStandardPaths>
#include <QAction>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QFile>
#include <QTime>
#include <QDate>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QPushButton>
#include <QFontDialog>
#include <QSpinBox>
#include <QWidget>
#include <QToolBar>
#include <QComboBox>
#include <QLabel>
#include <QFont>
#include <QColorDialog>

#include <QDebug>

#include "tracker.hpp"

Tracker::Tracker(QMainWindow* parent)
    : QMainWindow(parent)
    , _manager(new QNetworkAccessManager(this))
    , _toolbar(addToolBar("Toolbar"))
    , _settingsPath(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
    , _settings(_settingsPath + "/Guild Wars 2/event_tracker.ini", QSettings::IniFormat)
    , _timer(new QTimer(this))
{
    resize(200, 25);

    // http://stackoverflow.com/a/651345
    setContextMenuPolicy(Qt::PreventContextMenu);

    QList<QAction*> actions {
        new QAction("Quit", _toolbar),
        new QAction("Options", _toolbar),
        new QAction("Update", _toolbar),
        new QAction("Show all", _toolbar),
    };

    connect(actions[1], SIGNAL(triggered()), this, SLOT(openOptions()));
    connect(actions[2], SIGNAL(triggered()), this, SLOT(updateEvents()));

    actions[0]->connect(actions[0], &QAction::triggered, [=]() {
        saveSettings();
        close();
    });

    actions[3]->connect(actions[3], &QAction::triggered, [=]() {
        qDebug() << "Showing all events, size" << _wishlist.size();
        startTrackingEvents();
    });

    _toolbar->addActions(actions);

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    setWindowIcon(QIcon(":/icon.png"));

    connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(_timer, SIGNAL(timeout()), this, SLOT(updateEvents()));
}

Tracker::~Tracker() {
    delete _timer;

    // comboboxes parent window has set WA_DeleteOnClose attribute,
    // should handle deleting for us
    /*if (_langComboBox)
        delete _langComboBox;

    if (_servComboBox)
        delete _servComboBox;*/

    for (auto& el : _events)
        delete el;
    _events.clear();

    for (auto& el : _eventStates)
        delete el;
    _eventStates.clear();

    delete _manager;
}

void Tracker::getJsonData() {
    _requestCounter = 4;
    _manager->get(QNetworkRequest(QUrl(_baseUrl + "/v1/map_names.json?lang=" + _language)));
    _manager->get(QNetworkRequest(QUrl(_baseUrl + "/v1/world_names.json?lang=" + _language)));
    _manager->get(QNetworkRequest(QUrl(_baseUrl + "/v1/event_names.json?lang=" + _language)));
    _manager->get(QNetworkRequest(QUrl(_baseUrl + "/v1/events.json?world_id=" + _worldId)));
}

void Tracker::init(const QRect& desktop) {
    _eventColors["Active"]      = "#00FF00";
    _eventColors["Success"]     = "#999999";
    _eventColors["Fail"]        = "#FF0000";
    _eventColors["Warmup"]      = "#FFFFFF";
    _eventColors["Preparation"] = "#F4A460";

    if (!QFile::exists(_settingsPath + "/Guild Wars 2/event_tracker.ini"))
        firstTimeSetup(desktop);
    else
        loadSettings();

    getJsonData();

    saveSettings();
}

void Tracker::firstTimeSetup(const QRect& desktop) {
    move(desktop.width() - width(), desktop.height() - height() - 600);

    reloadEvents();

    openOptions();
}

void Tracker::saveSettings() {
    _settings.beginGroup("Tracker");
        _settings.setValue("size", size());
        _settings.setValue("position", pos());
        _settings.setValue("update_interval", _updateInterval);
        _settings.setValue("font", _font.toString());
        _settings.setValue("update_date", QDate::currentDate().toString("yyyy-MM-dd"));
    _settings.endGroup();

    _settings.beginGroup("Colors");
        for (const QString& key : _eventColors.keys())
            _settings.setValue(key, _eventColors[key]);
    _settings.endGroup();

    _settings.beginGroup("Game");
        _settings.setValue("language", _language);
        _settings.setValue("world_id", _worldId);
    _settings.endGroup();

    _settings.beginGroup("Events");
        int i = 0;
        for (const auto& list : _wishlist) {
            _settings.setValue("state" + QString::number(i), list.tracked);
            _settings.setValue("chain" + QString::number(i++), list);
        }
        _settings.setValue("size", i);
    _settings.endGroup();
}

void Tracker::loadSettings() {
    _settings.beginGroup("Tracker");
        resize(_settings.value("size").toSize());
        move(_settings.value("position").toPoint());
        _updateInterval = _settings.value("update_interval").toInt();
        _font.fromString(_settings.value("font").toString());
        setFont(_font);

        qDebug() << "last update date"
                 << _settings.value("update_date").toDate().toString("yyyy-MM-dd");

    _settings.endGroup();

    _settings.beginGroup("Colors");
        for (const QString& key : _eventColors.keys())
            _eventColors[key] = _settings.value(key).toString();
    _settings.endGroup();

    _settings.beginGroup("Game");
        _language = _settings.value("language").toString();
        _worldId = _settings.value("world_id").toString();
    _settings.endGroup();

    _settings.beginGroup("Events");
        int size = _settings.value("size").toInt();

        qDebug() << "loading" << size << "event chains...";

        for (int i = 0; i < size; ++i) {
            _wishlist.append(EventChain());
            _wishlist.last().tracked = _settings.value("state" + QString::number(i)).toBool();
            _wishlist.last().append(_settings.value("chain" + QString::number(i)).toStringList());
        }
    _settings.endGroup();

    qDebug() << "settings loaded, wishlist size" << _wishlist.size();
}

void Tracker::addEvents() {
    int index = 0;
    for (const auto& list : _wishlist) {
        int size = list.size();
        for (const QString& eid : list) {

            ClickableLabel* label = nullptr;

            if (_json["event_names"].contains(eid) && _eventStates.contains(eid)) {
                if (_eventStates[eid]->state == "Success" && size-- > 1)
                    continue;
                label = new ClickableLabel(_json["event_names"][eid], this);
            } else {
                label = new ClickableLabel("No info for " + eid, this);
            }

            // when restoring state (reopening app) check if chain is tracked
            if (!list.tracked)
                label->setStyleSheet("background-color: #000000");

            label->move(0, 25 + _events.size() * 25);
            label->setIndent(5);
            label->resize(200, 25);
            label->setAutoFillBackground(true);
            label->show();
            label->connect(label, &ClickableLabel::rightClicked, [=]() {
                label->setStyleSheet("background-color: #000000");
                //label->hide();
                _wishlist[index].tracked = false;

                qDebug() << "stopped tracking chain" << index;
            });

            _events.append(label);

            break;
        }
        ++index;
    }
    resize(200, 25 + _events.size() * 25);
}

void Tracker::reloadEvents() {
    for (unsigned i = 0; i < 7; ++i) {
        _wishlist.append(EventChain());
        _wishlist.last().tracked = true;
    }

    /**** BEHEMOTH (Queensdale) ****/
    _wishlist[0].append("CFBC4A8C-2917-478A-9063-1A8B43CC8C38");
    _wishlist[0].append("36330140-7A61-4708-99EB-010B10420E39");
    _wishlist[0].append("AFCF031A-F71D-4CEA-85E1-957179414B25");
    _wishlist[0].append("E539A5E3-A33B-4D5F-AEED-197D2716F79B");
    _wishlist[0].append("31CEBA08-E44D-472F-81B0-7143D73797F5");

    /**** FIRE ELEMENTAL (Metrica Province) ****/
    _wishlist[1].append("2C833C11-5CD5-4D96-A4CE-A74C04C9A278");
    _wishlist[1].append("5E4E9CD9-DD7C-49DB-8392-C99E1EF4E7DF");
    _wishlist[1].append("33F76E9E-0BB6-46D0-A3A9-BE4CDFC4A3A4");

    /**** Great jungle wurm (Caledon Forrest) ****/
    _wishlist[2].append("613A7660-8F3A-4897-8FAC-8747C12E42F8");
    _wishlist[2].append("1DCFE4AA-A2BD-44AC-8655-BBD508C505D1");
    _wishlist[2].append("456DD563-9FDA-4411-B8C7-4525F0AC4A6F");
    _wishlist[2].append("61BA7299-6213-4569-948B-864100F35E16");
    _wishlist[2].append("C5972F64-B894-45B4-BC31-2DEEA6B7C033");

    /**** Shatterer (Blazeridge Stepps) ****/
    _wishlist[3].append("580A44EE-BAED-429A-B8BE-907A18E36189");
    _wishlist[3].append("8E064416-64B5-4749-B9E2-31971AB41783");
    _wishlist[3].append("03BF176A-D59F-49CA-A311-39FC6F533F2F");

    /**** Tequatl () ****/
    _wishlist[4].append("568A30CF-8512-462F-9D67-647D69BEFAED");

    /**** Golem Mk II () ****/
    _wishlist[5].append("3ED4FEB4-A976-4597-94E8-8BFD9053522F");
    _wishlist[5].append("9AA133DC-F630-4A0E-BB5D-EE34A2B306C2");

    /**** Jormag () ****/
    _wishlist[6].append("0CA3A7E3-5F66-4651-B0CB-C45D3F0CAD95");
    _wishlist[6].append("BFD87D5B-6419-4637-AFC5-35357932AD2C");
    _wishlist[6].append("0464CB9E-1848-4AAA-BA31-4779A959DD71");
}

void Tracker::startTrackingEvents() {
    for (auto& list : _wishlist)
        list.tracked = true;

    _wishlist.clear();

    reloadEvents();

    for (auto& el : _events)
        delete el;
    _events.clear();

    addEvents();
    updateEvents();
}

/**************************************
 * PUBLIC SLOTS
 *************************************/
void Tracker::openOptions() {
    QWidget* window = new QWidget(nullptr);

    // http://stackoverflow.com/a/7944336
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->connect(window, &QWidget::destroyed, [=]() {
        saveSettings();
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        show();
    });

    window->setFocus();
    window->resize(300, 500);
    window->move(pos() - QPoint(150, 250));
    window->show();

    _servComboBox = new QComboBox(window);
    _servComboBox->resize(200, 25);
    _servComboBox->move(0, 0);
    _servComboBox->addItem("Select server...");

    QList<QString> list = _json["world_names"].keys();
    qSort(list);

    _servComboBox->addItems(list);
    _servComboBox->show();

    _langComboBox = new QComboBox(window);
    _langComboBox->resize(100, 25);
    _langComboBox->move(200, 0);
    _langComboBox->addItems(QStringList({"Language...", "English", "Deutsch", "Français", "Español"}));
    _langComboBox->show();

    _servComboBox->connect(_servComboBox, &QComboBox::currentTextChanged, [=](const QString& name) {
        if (!name.isEmpty() && !name.startsWith("Select")) {
            _worldId = _json["world_names"][name];
            qDebug() << "world_id set to" << _worldId;
        }
    });

    _langComboBox->connect(_langComboBox, &QComboBox::currentTextChanged, [=](const QString& lang) {
        if (!lang.startsWith("Language")) {
            _language = lang.left(2).toLower();
            qDebug() << "language set to" << _language;
        }
        getJsonData();
    });

    QLabel* intervalLabel = new QLabel("Update interval (in seconds)", window);
    intervalLabel->resize(150, 25);
    intervalLabel->move(0, 50);
    intervalLabel->setIndent(5);
    intervalLabel->show();

    QSpinBox* intervalPicker = new QSpinBox(window);
    intervalPicker->setRange(10, 600); // 10 sec, 10 min
    intervalPicker->resize(50, 25);
    intervalPicker->move(150, 50);
    intervalPicker->show();

    // http://lists.qt-project.org/pipermail/interest/2012-September/003894.html
    void (QSpinBox:: *signal)(int) = &QSpinBox::valueChanged;
    intervalPicker->connect(intervalPicker, signal, [=](int i) {
        _updateInterval = i;
        _timer->setInterval(_updateInterval * 1000);
    });

    QPushButton* button = new QPushButton("Change font", window);
    button->resize(100, 25);
    button->move(200, 50);
    button->show();

    button->connect(button, &QPushButton::clicked, [=]() {
        bool ok;
        _font = QFontDialog::getFont(&ok, _font, this);

        if (ok)
            setFont(_font);
    });

    int i = 0;
    for (const QString& key : _eventColors.keys()) {
        QPushButton* btn = new QPushButton("Change color for " + key, window);
        btn->resize(200, 25);
        btn->move(0, 100 + i++ * 25);
        btn->show();
        btn->connect(btn, &QPushButton::clicked, [=]() {
            _eventColors[key] = QColorDialog::getColor(_eventColors[key], window).name();
        });
    }

    // disable top most window property so options window can get focus
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);
    show();
}

void Tracker::updateEvents() {
    if (QTime::currentTime().hour() >= 2 &&
        QDate::currentDate() > _settings.value("Tracker/update_date").toDate()) {
        qDebug() << "MUST FORCE UPDATE TODAY";
        _settings.setValue("Tracker/update_date", QDate::currentDate().toString("yyyy-MM-dd"));
        startTrackingEvents();
    } else
        qDebug() << "don't have to force update today";

    _manager->get(QNetworkRequest(QUrl(_baseUrl + "/v1/events.json?world_id=" + _worldId)));

    /**
     * Active      – The event is running now
     * Success     – The event has succeeded
     * Fail        – The event has failed
     * Warmup      – The event is inactive, and will only become active once certain criteria are met
     * Preparation – The criteria for the event to start have been met, but certain activities
     * (such as an NPC dialogue) have not completed yet. After the activites have been completed,
     * the event will become Active.
     */
    int x = 1;
    int index = 0;
    for (const auto& list : _wishlist) {
        int size = list.size();
        int y = 1;

        if (!list.tracked) {
            //qDebug() << "SKIPPING UPDATE FOR" << list;
            ++index;
            continue;
        }

        for (const QString& id : list) {
            if (!_eventStates.contains(id)) {
                qDebug() << "update :: can't find key" << id;
                continue;
            }

            if (_eventStates[id]->state == "Success" && size-- > 1) {
                //qDebug() << x << "/" << y++ << _eventStates[id]->state << "SKIPPING" <<_json["event_names"][id];
                continue;
            } else {
                qDebug() << x << "/" << y++ << _eventStates[id]->state << _json["event_names"][id];
            }

            QPalette palette;
            palette.setColor(QPalette::Window, _eventColors[_eventStates[id]->state]);

            _events[index]->setText(_json["event_names"][id]);
            _events[index]->setPalette(palette);

            break;
        }
        x++;
        ++index;
    }
}

void Tracker::replyFinished(QNetworkReply* reply) {
    QJsonDocument doc = QJsonDocument::fromJson(QString(reply->readAll()).toUtf8());
    QJsonArray array = doc.array();

    QString key = reply->url().path().mid(4, reply->url().path().indexOf('.') - 4);

    if (key == "events") {
        array = doc.object()["events"].toArray();
        for (QJsonValue value : array) {
            QJsonObject object = value.toObject();
            QString event_id = object["event_id"].toString();
            QString map_id = object["map_id"].toString();
            QString state = object["state"].toString();

            // update if we already have event state
            if (_eventStates.contains(event_id)) {
                _eventStates[event_id]->map_id = map_id;
                _eventStates[event_id]->state = state;
            } else
                _eventStates[event_id] = new EventState(map_id, state);
        }
        qDebug() << key << _eventStates.size();
    // world_names, map_names, event_names
    } else {
        for (QJsonValue value : array) {
            QJsonObject object = value.toObject();
            QString id = object["id"].toString();
            QString name = object["name"].toString();

            if (key == "world_names")
                _json[key][name] = id;
            else
                _json[key][id] = name;
        }
        qDebug() << key << _json[key].size();
    }

    if (--_requestCounter == 0 && !_timer->isActive()) {
        _timer->start(_updateInterval * 1000);
        addEvents();
        updateEvents();
    }
}
