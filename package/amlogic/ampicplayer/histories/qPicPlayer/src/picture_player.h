#ifndef PICTURE_PLAYER_H
#define PICTURE_PLAYER_H

#include <QGraphicsPixmapItem>
#include <QDesktopWidget>
#include <QImage>
#include <QImageReader>
#include "cpictransition.h"
#include "qusrgraphicsview.h"

#ifdef __linux__
#define USE_FULL_SCREEN
#define USE_SYSTEM_DBUS
#endif

#define MAX_PREPARE_SCENE_NUMBER    3

class picture_player : public QGraphicsView
{
    Q_OBJECT

public:
    picture_player(int screen_width, int screen_height, QGraphicsView *parent = 0);
    ~picture_player();

public Q_SLOTS:
    void createPicture(int id, const QString &pictureURL, int x, int y, int w, int h, const QString &parameter);
    void createScene(int id, int x, int y, int w, int h);
    void displayScene(int id, const QString &transitionName, int args);
    void adjustDisplayScene(int degree, int scale_x, int scale_y, int center_x, int center_y, const QString &parameter);
    void setDesktop(int w, int h);
    int ScenceStatus(int id);

private:
	QRect display_area;
	QRect prepare_area;
    QGraphicsScene *display_scene;
    QUsrGraphicsView *prepare_view[MAX_PREPARE_SCENE_NUMBER];
    QGraphicsScene *prepare_scene[MAX_PREPARE_SCENE_NUMBER];
    qreal scaling_x;
    qreal scaling_y;
    unsigned current_degree;
    CPicTransition pic_transiion;
};

#endif // PICTURE_PLAYER_H
