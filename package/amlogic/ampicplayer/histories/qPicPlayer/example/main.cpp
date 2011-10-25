#include <QtGui/QApplication>
#include <QtDBus/QtDBus>
#include <QDesktopWidget>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDesktopWidget *desktop = QApplication::desktop();
	int screen_width,screen_height;
#ifdef USE_FULL_SCREEN
/*******************************************************************
* The frame buffer is split into two parts. The left one is for shown. 
* and the right one is for calculating. so you should en large your
* frame buffer to 2*Width X height to support the effect. 
*********************************************************************/
	screen_width=desktop->width()/2;
	screen_height=desktop->height();
#else
	screen_width=desktop->width()/4;
	screen_height=desktop->height()/2;
#endif
	MainWindow w(screen_width, screen_height);
#ifdef USE_SYSTEM_DBUS
    if (!QDBusConnection::systemBus().isConnected()) {
#else
    if (!QDBusConnection::sessionBus().isConnected()) {
#endif
        qWarning("Cannot connect to the D-Bus session bus.\n");
        return 1;
    }

    w.show();
    return a.exec();
}
