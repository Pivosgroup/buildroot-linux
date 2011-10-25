#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtGui/QMainWindow>

#ifdef __linux__
#define USE_FULL_SCREEN
#define USE_SYSTEM_DBUS
#else
#define PIC_FOLDER_SELECT
#endif

class PicturePlayerInterface;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int screen_width, int screen_height, QWidget *parent = 0);
    ~MainWindow();

protected:
    void timerEvent(QTimerEvent *event);
    void openDirectory();

private:
    PicturePlayerInterface *picture_player;
    int index;
    QStringList photos;
    int scene_width;
    int scene_height;
    int degree;
    int scale;
    int prepared;
    int last_prepared;
    qreal last_ratio;
};

#endif // MAINWINDOW_H
