#include <QtGui/QApplication>
#include <QtDBus/QtDBus>
#include "picture_player.h"
#include "picture_player_adapter.h"
#include <QWSServer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifndef QT_NO_CURSOR
    QWSServer::setCursorVisible(false);
#endif
    QDesktopWidget *desktop = QApplication::desktop();
#ifdef USE_FULL_SCREEN
    picture_player w(desktop->width()/2, desktop->height());
#else
    picture_player w(desktop->width()/4, desktop->height()/2);
#endif
    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    w.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    w.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    w.setFrameShape(QFrame::NoFrame);
    w.show();

#ifdef USE_SYSTEM_DBUS
    if (!QDBusConnection::systemBus().isConnected()) {
#else
    if (!QDBusConnection::sessionBus().isConnected()) {
#endif
        qWarning("Cannot connect to the D-Bus session bus.\n");
        return 1;
    }

#ifdef USE_SYSTEM_DBUS
    QDBusConnection connection =  QDBusConnection::systemBus();
#else
    QDBusConnection connection =  QDBusConnection::sessionBus();
#endif
    new PicturePlayerInterfaceAdaptor(&w);
    if (connection.isConnected()){
        if (!connection.registerObject("/Picture", &w)){
            qWarning("Cannot register object to the D-Bus session bus.\n");
            return 1;
        }
        if (!connection.registerService("com.amlogic.player")){
            qWarning("Cannot register service to the D-Bus session bus.\n");
            return 1;
        }
    }
    else{
        qWarning("Cannot connect to the D-Bus session bus.\n");
        return 1;
    }
    return a.exec();
}
