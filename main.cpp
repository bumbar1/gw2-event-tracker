#include <QApplication>
#include <QDesktopWidget>

#include "tracker.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Tracker tracker;
    tracker.init(app.desktop()->screenGeometry());
    tracker.show();

    return app.exec();
}
