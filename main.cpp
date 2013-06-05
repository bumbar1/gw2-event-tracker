#include <QApplication>
#include <QDesktopWidget>

#include "tracker.hpp"

//#include <windows.h>
//#include <iostream>

int main(int argc, char *argv[]) {

    /*AllocConsole();
    freopen("conin$",  "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);*/

    //std::cout << "initilizing app...\n";
    QApplication app(argc, argv);

    //std::cout << "starting tracker...\n";
    Tracker tracker;
    tracker.init(app.desktop()->screenGeometry());
    tracker.show();

    //std::cout << "closing main...\n";
    return app.exec();
}
